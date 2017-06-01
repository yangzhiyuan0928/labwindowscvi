#include <Windows.h>
#include <cvi2009compat.h>   
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include <rs232.h>
#include <utility.h>
#include <formatio.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Terminal.h"  

static int panelHandle;

typedef struct portInfo
{
	int port_open; //端口打开标志位
	int RS232Error;
	int portNum;
	int portIndex;
	char deviceName[20];
} portInfo, *portInfoPtr;
//初始化结构体
portInfo uartPort     = {0,-1,0,0,""};
portInfo traffic1Port = {0,-1,0,0,""};   
portInfo traffic2Port = {0,-1,0,0,""};   

//流量计数据获取命令编码
static unsigned char protocolSet[6]         = {0x01,0x67,0xAA,0x55,0x9A,0x0D}; //485协议切换到modbus协议
static unsigned char baudSet[8]             = {0x01,0x06,0x00,0x15,0x00,0x01,0x59,0xCE}; //设定波特率为9600bps 
static unsigned char addressSet[8]          = {0x01,0x06,0x00,0x01,0x00,0x01,0x19,0xCA}; //流量计通信地址设置为1
static unsigned char totTrafficReg4clear[8] = {0x01,0x06,0x00,0x04,0x00,0x00,0xC8,0x0B}; //总流量清零
static unsigned char totTrafficReg5clear[8] = {0x01,0x06,0x00,0x05,0x00,0x00,0x99,0xCB};
static unsigned char totTrafficReg6clear[8] = {0x01,0x06,0x00,0x06,0x00,0x00,0x69,0xCB};
static unsigned char totTraffic[8]          = {0x01,0x03,0x00,0x04,0x00,0x03,0x44,0x0A}; //读取总流量
static unsigned char curTraffic[8]          = {0x01,0x03,0x00,0x02,0x00,0x02,0x65,0xCB}; //读取瞬时流量

//Internal function prototypes
void DisplayRS232Error(void);
void SendByte(portInfo port,char ch);
double Compoundat(int dat1,int dat2); //将两个int数据合成一个double数据:一个int是整数部分，一个int是小数点后面的数据
	
//Uart回调函数
void CVICALLBACK UartComCallback(int portNo, int eventMask, void *callbackData);
void CVICALLBACK Traffic1ComCallback(int portNo, int eventMask, void *callbackData);
void CVICALLBACK Traffic2ComCallback(int portNo, int eventMask, void *callbackData);
	
int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Terminal.uir", PANEL)) < 0)
		return -1;
	
	//初始化端口选择、LED图案选择、状态指示灯
	SetCtrlAttribute(panelHandle,PANEL_UART_LED,ATTR_OFF_COLOR,VAL_RED);
	SetCtrlAttribute(panelHandle,PANEL_UART_LED,ATTR_ON_COLOR,VAL_GREEN);
	
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC_LED_1,ATTR_OFF_COLOR,VAL_RED);
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC_LED_1,ATTR_ON_COLOR,VAL_GREEN);
	
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC_LED_2,ATTR_OFF_COLOR,VAL_RED);
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC_LED_2,ATTR_ON_COLOR,VAL_GREEN);
	
	//关闭定时器
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1  
	SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2

	
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}											  


int CVICALLBACK panelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
	}
	return 0;
}

//配置uart
int CVICALLBACK PortEnable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:  //注意：多次打开串口会出错
			uartPort.port_open = 0; //未打开
			GetCtrlVal(panelHandle,PANEL_PORT_NUM,&uartPort.portNum); //获取端口号
			GetCtrlIndex(panelHandle,PANEL_PORT_NUM,&uartPort.portIndex);
			GetLabelFromIndex(panelHandle,PANEL_PORT_NUM,uartPort.portIndex,uartPort.deviceName);
			DisableBreakOnLibraryErrors();	 
			uartPort.RS232Error = OpenComConfig(uartPort.portNum,uartPort.deviceName,115200,0,8,1,512,512); //STM32控制，打开串口
			EnableBreakOnLibraryErrors();
			if(uartPort.RS232Error == 0)  //串口打开成功
			{
				uartPort.port_open = 1;
				SetXMode(uartPort.portNum,0); //关闭XOFF
				SetCTSMode(uartPort.portNum,0);
				SetComTime(uartPort.portNum,5);
				SetCtrlVal(panelHandle,PANEL_UART_LED,1); //点亮指示灯
				//清空串口的TxBuffer、RxBuffer
				FlushInQ(uartPort.portNum);
				FlushOutQ(uartPort.portNum);
				
				//建立串口回调函数
				InstallComCallback(uartPort.portNum,LWRS_RXFLAG|LWRS_RECEIVE,1,10,UartComCallback,0);  //以回车为标志位，接收到回车就产生中断，调用中断函数UartComCall.
			}
			else
			{
				MessagePopup("Error","该COM口已被占用，请选择其他COM口！");
				SetCtrlVal(panelHandle,PANEL_UART_LED,0); //关闭指示灯
				uartPort.port_open = 0;
			}
			break;
	}
	return 0;
}

//流量计1对应的uart端口配置
int  CVICALLBACK Traffic1PortEnablde(int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:  
			traffic1Port.port_open = 0; //未打开
			GetCtrlVal(panelHandle,PANEL_TRAFFIC1_NUM,&traffic1Port.portNum); //获取端口号
			GetCtrlIndex(panelHandle,PANEL_TRAFFIC1_NUM,&traffic1Port.portIndex);
			GetLabelFromIndex(panelHandle,PANEL_TRAFFIC1_NUM,traffic1Port.portIndex,traffic1Port.deviceName);
			DisableBreakOnLibraryErrors();	 
			traffic1Port.RS232Error = OpenComConfig(traffic1Port.portNum,traffic1Port.deviceName,38400,0,8,1,512,512); //打开串口
			EnableBreakOnLibraryErrors();
			if(traffic1Port.RS232Error == 0)  //串口打开成功
			{
				traffic1Port.port_open = 1;
				SetXMode(traffic1Port.portNum,0); //关闭XOFF
				SetCTSMode(traffic1Port.portNum,0);
				SetComTime(traffic1Port.portNum,5);
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_1,1); //点亮指示灯
				//清空串口的TxBuffer、RxBuffer
				FlushInQ(traffic1Port.portNum);
				FlushOutQ(traffic1Port.portNum); 
				//建立串口回调函数：接收中断函数
				InstallComCallback(traffic1Port.portNum,LWRS_RECEIVE,20,0,Traffic1ComCallback,0);
			}
			else
			{
				MessagePopup("Error","该COM口已被占用，请选择其他COM口！");
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_1,0); //关闭指示灯
				traffic1Port.port_open = 0;
			}
			break;
	}
	return 0;
}


//流量计2对应的uart端口配置
int  CVICALLBACK Traffic2PortEnablde(int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:  
			traffic2Port.port_open = 0; //未打开
			GetCtrlVal(panelHandle,PANEL_TRAFFIC2_NUM,&traffic2Port.portNum); //获取端口号
			GetCtrlIndex(panelHandle,PANEL_TRAFFIC2_NUM,&traffic2Port.portIndex);
			GetLabelFromIndex(panelHandle,PANEL_TRAFFIC2_NUM,traffic2Port.portIndex,traffic2Port.deviceName);
			DisableBreakOnLibraryErrors();	 
			traffic2Port.RS232Error = OpenComConfig(traffic2Port.portNum,traffic2Port.deviceName,38400,0,8,1,512,512); //打开串口
			EnableBreakOnLibraryErrors();
			if(traffic2Port.RS232Error == 0)  //串口打开成功
			{
				traffic2Port.port_open = 1;
				SetXMode(traffic2Port.portNum,0); //关闭XOFF
				SetCTSMode(traffic2Port.portNum,0);
				SetComTime(traffic2Port.portNum,5);
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_2,1); //点亮指示灯
				//清空串口的TxBuffer、RxBuffer
				FlushInQ(traffic2Port.portNum);
				FlushOutQ(traffic2Port.portNum); 
				//建立串口回调函数
				InstallComCallback(traffic2Port.portNum,LWRS_RECEIVE,20,0,Traffic2ComCallback,0);     
			}
			else
			{
				MessagePopup("Error","该COM口已被占用，请选择其他COM口！");
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_2,0); //关闭指示灯
				traffic2Port.port_open = 0;
			}
			break;
	}
	return 0;
}


int CVICALLBACK PortClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(uartPort.port_open == 0)
				MessagePopup("Error","串口还未打开！");
			else
			{
				SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1  
				SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2 
				
				uartPort.port_open = 0;
				CloseCom(uartPort.portNum); //关闭串口
				SetCtrlVal(panelHandle,PANEL_UART_LED,0); //关闭指示灯 
				
				if(traffic1Port.port_open == 1)
					CloseCom(traffic1Port.portNum); //关闭流量计1串口
				traffic1Port.port_open = 0;
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_1,0);
				
				if(traffic2Port.port_open == 1)
					CloseCom(traffic2Port.portNum);
				traffic2Port.port_open = 0;
				SetCtrlVal(panelHandle,PANEL_TRAFFIC_LED_2,0); 
			}
			break;
	}
	return 0;
}

//LED矩阵显示图像选择
int  CVICALLBACK LedGraphSet(int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int ledColor = 0; //0代表绿色、1代表蓝色
	int ledDir = 0;   //0代表上下移动，1代表左右移动
	int ledNum = 0;   //对应点亮的LED个数
	switch (event)
	{
		case EVENT_COMMIT:
			if(uartPort.port_open == 0)
				MessagePopup("Error","串口还未打开！");
			else
			{
				GetCtrlVal(panelHandle,PANEL_COLOR_SWITCH,&ledColor); 
				GetCtrlVal(panelHandle,PANEL_DIR_SWITCH,&ledDir);
				GetCtrlVal(panelHandle,PANEL_NUM_SELE,&ledNum);
				//判断LED图案设置是否合理
				if(ledDir == 1 && ledNum > 5 && ledNum < 8)  //左右方向移动时，LED最多可设置10个亮
					MessagePopup("Error","左右方向上最多可点亮10个LED，请重新设置LED个数！");
				else switch(ledColor)
				{
					case 0: //蓝色 - 0
						SendByte(uartPort,0x00);
						switch(ledDir)
						{
							case 0: //上下 - 0
								SendByte(uartPort,0x00);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00); 
										break;
								}
								break;  
							case 1:  //左右 - 1
								SendByte(uartPort,0x01);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00);
										break;
										
								}
								break;  
							default:  //默认全关
								SendByte(uartPort,0x00);  //
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00);
										break;
								}
								break; 
						}
						break;
					
					case 1: //绿色
						SendByte(uartPort,0x01);
						switch(ledDir)
						{
							case 0: //上下 - 0
								SendByte(uartPort,0x00);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00); 
										break;
								}
								break;  
							case 1:  //左右 - 1
								SendByte(uartPort,0x01);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00);
										break;
										
								}
								break;  
							default:  //默认全关
								SendByte(uartPort,0x00);  //
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00); 
										break;
								}
								break; 
						}
						break;
						
					default:  
						SendByte(uartPort,0x00);
						switch(ledDir)
						{
							case 0: //上下 - 0
								SendByte(uartPort,0x00);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00); 
										break;
								}
								break;  
							case 1:  //左右 - 1
								SendByte(uartPort,0x01);
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00);
										break;
									
								}
								break;  
							default:  //默认全关
								SendByte(uartPort,0x00);  //
								switch(ledNum)
								{
									case 0: //全关 - A5
										SendByte(uartPort,0x00);	
										break;
									case 1: //2 LED
										SendByte(uartPort,0x01);
										break;
									case 2: //4 LED
										SendByte(uartPort,0x02);
										break;
									case 3: //6 LED
										SendByte(uartPort,0x03);
										break;
									case 4: //8 LED
										SendByte(uartPort,0x04);
										break;
									case 5: //10 LED
										SendByte(uartPort,0x05);
										break;
									case 6: //12 LED
										SendByte(uartPort,0x06);
										break;
									case 7: //14 LED
										SendByte(uartPort,0x07);
										break;
									case 8: //ALL ON
										SendByte(uartPort,0x08);
										break;
									default: //ALL OFF
										SendByte(uartPort,0x00); 
										break;
								}
								break; 
						}
						break;
				}

			}
			break;
	}
	return 0;
}

//电磁阀1控制开关
int CVICALLBACK ValveCtrl_1 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int valve1_State = 0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(uartPort.port_open == 0)
				MessagePopup("Error","串口还未打开！");
			else
			{
				GetCtrlVal(panelHandle,PANEL_VALVE1_CTRL,&valve1_State);
				switch(valve1_State)
				{
					case 0: //电磁阀关闭时：关闭流量计1对应的定时器
						SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1 
						SendByte(uartPort,'J');  
						break;
					case 1:
						if(traffic1Port.port_open == 1)  //判断流量计1对应串口是否打开
						{
							SendByte(uartPort,'H'); //开电磁阀(流量计串口要先打开)时：设置流量计通信协议485转modbus，设置波特率，设置通信地址，，使能流量计1对应的定时器
							SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,1); //开启定时器1
						}
						else 
						{
							SetCtrlVal(panelHandle,PANEL_VALVE1_CTRL,0);  //将前面板上控制开关拨到OFF状态  
							MessagePopup("Error","流量计1对应的串口还未打开");
						}
						break;
					default:
						SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1 
						SendByte(uartPort,'J');   
						break;
				}
			}
			break;
	}
	return 0;
}

//电磁阀2控制开关
int CVICALLBACK ValveCtrl_2 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int valve2_State = 0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(uartPort.port_open == 0)
				MessagePopup("Error","串口还未打开！");
			else													  
			{
				GetCtrlVal(panelHandle,PANEL_VALVE2_CTRL,&valve2_State);  
				switch(valve2_State)
				{
					case 0: 
						SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2   
						SendByte(uartPort,'K');    
						break;
					case 1:
						if(traffic2Port.port_open == 1)
						{
							SendByte(uartPort,'I'); 
							SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,1); //开启定时器2 
						}
						else
							MessagePopup("Error","流量计1对应的串口还未打开");   
						break;
					default:
						SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2           
						SendByte(uartPort,'K');   //电磁阀关闭时：关闭流量计2对应的定时器
						break;
				}
			}
			break;
	}
	return 0;
}

//流量计1总流量清零
int  CVICALLBACK Traffic1_Clear(int panel, int control, int event, 
	void *callbackData, int eventData1, int eventData2)
{
	char readBuf[256] = {0};
	int strLen = 0;
	int timerState = 0;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,&timerState);   
			SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1
			if(traffic1Port.port_open == 1)
			{
				FlushOutQ(traffic1Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic1Port.portNum); //清空接收缓存区
				ComWrt(traffic1Port.portNum,totTrafficReg4clear,sizeof(totTrafficReg4clear));//总流量清零
				while(GetInQLen(traffic1Port.portNum) < 8);
				ComRd(traffic1Port.portNum,readBuf,8);  
					
				FlushOutQ(traffic1Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic1Port.portNum); //清空接收缓存区
				ComWrt(traffic1Port.portNum,totTrafficReg5clear,sizeof(totTrafficReg5clear));//总流量清零
				while(GetInQLen(traffic1Port.portNum) < 8);
				ComRd(traffic1Port.portNum,readBuf,8);  
					
				FlushOutQ(traffic1Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic1Port.portNum); //清空接收缓存区
			    ComWrt(traffic1Port.portNum,totTrafficReg6clear,sizeof(totTrafficReg6clear));//总流量清零
				while(GetInQLen(traffic1Port.portNum) < 8);
				ComRd(traffic1Port.portNum,readBuf,8);  
				
				MessagePopup("Message","总流量清零成功！");	
				SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,timerState); //恢复定时器1
			}
			else
				MessagePopup("Error","流量计1对应的串口还未打开！");
			break;
	}
	return 0;
}

//流量计2总流量清零
int  CVICALLBACK Traffic2_Clear(int panel, int control, int event, 
	void *callbackData, int eventData1, int eventData2)
{
	char readBuf[256] = {0};
	int strLen = 0;
	int timerState = 0; 
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,&timerState);             
			SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2           
			if(traffic2Port.port_open == 1)
			{
				FlushOutQ(traffic2Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic2Port.portNum); //清空接收缓存区
				ComWrt(traffic2Port.portNum,totTrafficReg4clear,sizeof(totTrafficReg4clear));//总流量清零
				while(GetInQLen(traffic2Port.portNum) < 8);
				ComRd(traffic2Port.portNum,readBuf,8);  
					
				FlushOutQ(traffic2Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic2Port.portNum); //清空接收缓存区
				ComWrt(traffic2Port.portNum,totTrafficReg5clear,sizeof(totTrafficReg5clear));//总流量清零
				while(GetInQLen(traffic2Port.portNum) < 8);
				ComRd(traffic2Port.portNum,readBuf,8);  
					
				FlushOutQ(traffic2Port.portNum);  //清空发送缓存区 
				FlushInQ(traffic2Port.portNum); //清空接收缓存区
			    ComWrt(traffic2Port.portNum,totTrafficReg6clear,sizeof(totTrafficReg6clear));//总流量清零
				while(GetInQLen(traffic2Port.portNum) < 8);
				ComRd(traffic2Port.portNum,readBuf,8); 
				
				MessagePopup("Message","总流量清零成功！");		
				SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,timerState); //开启定时器2 
			}
			else
				MessagePopup("Error","流量计2对应的串口还未打开");
			break;
	}
	return 0;
}

//两个uint_16数据合成一个double
double Compoundat(int dat1,int dat2)
{
	double data = 0.0;
	if(dat2>10000)  //dat2最大值为65535
		data = dat1 + (double)dat2/100000;
	else if(dat2>1000 && dat2<10000)
		data = dat1 + (double)dat2/10000;
	else if(dat2>100 && dat2<1000)
		data = dat1 + (double)dat2/1000;
	else if(dat2>10 && dat2<100)
		data = dat1 + (double)dat2/100;
	else
		data = dat1 + (double)dat2/10;
	return data;
}

//退出回调函数
int CVICALLBACK QuitCallBack (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}												
	return 0;
}

//流量计1对应的定时器
int  CVICALLBACK TrafficTimer1(int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_TIMER_TICK: //定时间隔200ms
			FlushInQ(traffic1Port.portNum);  //清空接收缓存区     
			FlushOutQ(traffic1Port.portNum);  //清空发送缓存区  
			ComWrt(traffic1Port.portNum,totTraffic,sizeof(totTraffic));   //发送查询流量计1总流量命令
			Delay(0.010);  //延时10ms
			ComWrt(traffic1Port.portNum,curTraffic,sizeof(curTraffic));   //发送查询流量计1总流量命令
			break;
	}
	return 0;
}


//流量计2对应的定时器
int  CVICALLBACK TrafficTimer2(int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_TIMER_TICK: //定时间隔50ms
			FlushInQ(traffic2Port.portNum); //清空接收缓存区
			FlushOutQ(traffic2Port.portNum);  //清空发送缓存区  
			ComWrt(traffic2Port.portNum,totTraffic,sizeof(totTraffic));  //发送查询流量计2总流量命令  
			Delay(0.010);  //延时10ms 
			ComWrt(traffic2Port.portNum,curTraffic,sizeof(curTraffic));  //发送查询流量计2总流量命令
			break;
	}
	return 0;
}


//发送一个char数据
void SendByte(portInfo port,char ch)
{
	if(port.port_open == 0)
		MessagePopup("Error","串口还未打开！");
	else
	{
//		FlushOutQ(port.portNum);  //清空发送缓存区
		ComWrt(port.portNum,&ch,sizeof(ch));
	}
}


void CVICALLBACK UartComCallback(int portNo, int eventMask, void *callbackData)
{
	char readBuf[256] = {0};
	int strLen;
	strLen = GetInQLen(uartPort.portNum);
	ComRd(uartPort.portNum,readBuf,strLen);	
	SetCtrlVal(panelHandle,PANEL_RECEIVE_MESSAGE,readBuf);
}

void CVICALLBACK Traffic1ComCallback(int portNo, int eventMask, void *callbackData)
{
	double totalTraffic = 0.0;
	double traffic = 0.0;
	double trafficSpeed = 0.0;
	unsigned int regVal_02 = 0;
	unsigned int regVal_03 = 0;
	unsigned int regVal_04 = 0;
	unsigned int regVal_05 = 0;
	unsigned int regVal_06 = 0;
	unsigned char readBuf[256] = {0};
	int strLen = 0;
	
	strLen = GetInQLen(traffic1Port.portNum); // 20
	ComRd(traffic1Port.portNum,readBuf,strLen);
	regVal_04 = readBuf[3]*256 + readBuf[4];
	regVal_05 = readBuf[5]*256 + readBuf[6];
	regVal_06 = readBuf[7]*256 + readBuf[8];
	totalTraffic = Compoundat((regVal_04*65536+regVal_05),regVal_06); //单位是mL
	//printf("总流量：%lf mL\n",totalTraffic); //打印结果 
	PlotStripChartPoint(panelHandle,PANEL_FLOW1METRECHART_1,totalTraffic); //显示数据 
	GetCtrlVal(panelHandle,PANEL_TRAFFIC_1,&traffic);
	if(totalTraffic>=traffic)
	{
		SetCtrlAttribute(panelHandle,PANEL_TRAFFIC1_TIMER,ATTR_ENABLED,0); //关闭定时器1  
		SendByte(uartPort,'J');  //关闭电磁阀1
		SetCtrlVal(panelHandle,PANEL_VALVE1_CTRL,0);  //将前面板上控制开关拨到OFF状态
		MessagePopup("Message","总流量已达到设定值！");
	}
	
	regVal_02 = readBuf[14]*256 + readBuf[15];
	regVal_03 = readBuf[16]*256 + readBuf[17];
	trafficSpeed = (double)(regVal_02*65536 + regVal_03)/1000; //
	PlotStripChartPoint(panelHandle,PANEL_TRAFFICSPEEDCHART_1,trafficSpeed); //显示流速数据   

}

void CVICALLBACK Traffic2ComCallback(int portNo, int eventMask, void *callbackData)
{
	double totalTraffic = 0.0;
	double traffic = 0.0;
	double trafficSpeed = 0.0;
	unsigned int regVal_02 = 0;
	unsigned int regVal_03 = 0;
	unsigned int regVal_04 = 0;
	unsigned int regVal_05 = 0;
	unsigned int regVal_06 = 0;
	unsigned char readBuf[256] = {0};
	int strLen = 0;
	strLen = GetInQLen(traffic2Port.portNum); //20
	ComRd(traffic2Port.portNum,readBuf,strLen);
	regVal_04 = readBuf[3]*256 + readBuf[4];
	regVal_05 = readBuf[5]*256 + readBuf[6];
	regVal_06 = readBuf[7]*256 + readBuf[8];
	totalTraffic = Compoundat((regVal_04*65536+regVal_05),regVal_06); //单位是mL
	PlotStripChartPoint(panelHandle,PANEL_FLOW1METRECHART_2,totalTraffic); //显示数据 
	GetCtrlVal(panelHandle,PANEL_TRAFFIC_2,&traffic);
	if(totalTraffic>=traffic)
	{
		SetCtrlAttribute(panelHandle,PANEL_TRAFFIC2_TIMER,ATTR_ENABLED,0); //关闭定时器2     				
		SendByte(uartPort,'K');  //关闭电磁阀1
		SetCtrlVal(panelHandle,PANEL_VALVE2_CTRL,0);  //将前面板上控制开关拨到OFF状态  
		MessagePopup("Message","总流量已达到设定值！");  
	}	 
	
	regVal_02 = readBuf[14]*256 + readBuf[15];
	regVal_03 = readBuf[16]*256 + readBuf[17];
	trafficSpeed = (double)(regVal_02*65536 + regVal_03)/1000; //
	PlotStripChartPoint(panelHandle,PANEL_TRAFFICSPEEDCHART_2,trafficSpeed); //显示流速数据   

}
