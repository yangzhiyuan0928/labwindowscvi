#include <tcpsupp.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include "progressbar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utility.h>
#include <errno.h>
#include <ansi_c.h>
#include <cvirte.h>
#include <userint.h>
#include "flusample.h"
#include "data_process.h"

static int panelHandle;
static int TabPanel_1;

//TCP Config
extern int g_hconversation = 0;  //TCP Cmd handle
extern int d_hconversation = 0;  //TCP Data handle
static int serverRegisted = 0;

char parampath[NAME_LEN]; //参数配置
char dataDir[NAME_LEN] = {0};  
paramSet parameter = {1,50,20000,50,1,10000,1,2,1,0,0,2,1,5,7,56,56};  //面板参数结构体   
paramSet param; //下位机回发的参数
unsigned int fcnt = 0;    //AD采样数据存储文件计数
unsigned int ftotal = 0;

int paramsetted = 0;  //参数配置完成标志
int dirselected = 0;  //上传文件路径选择

//TCP Server相关回调函数
static int CVICALLBACK ServerCallback_CMD (unsigned int handle, int xType,int errCode, void *cbData);
static int CVICALLBACK ServerCallback_DAT (unsigned int handle, int xType,int errCode, void *cbData);

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Fluctuate_Sample.uir", PANEL)) < 0)
		return -1;

	//初始化Tab控件活动面板
	SetActiveTabPage (panelHandle, PANEL_TAB, 0);
	GetPanelHandleFromTabPage (panelHandle, PANEL_TAB, 0, &TabPanel_1);  //获取TabHandle

	//初始化各状态指示灯
	SetCtrlAttribute (panelHandle, PANEL_TCP_LED, ATTR_OFF_COLOR,VAL_RED); 		
	SetCtrlAttribute (panelHandle, PANEL_TCP_LED, ATTR_ON_COLOR,VAL_GREEN); 	
	SetCtrlAttribute (panelHandle, PANEL_DATA_LED, ATTR_OFF_COLOR,VAL_RED); 	
	SetCtrlAttribute (panelHandle, PANEL_DATA_LED, ATTR_ON_COLOR,VAL_GREEN); 	
	SetCtrlAttribute (TabPanel_1, TABPANEL1_PARAM_CONFIG_LED, ATTR_OFF_COLOR,VAL_RED); 	
	SetCtrlAttribute (TabPanel_1, TABPANEL1_PARAM_CONFIG_LED, ATTR_ON_COLOR,VAL_GREEN); 
	
	//初始化进度条
	ProgressBar_ConvertFromSlide (TabPanel_1,TABPANEL1_PROGRESS);
	ProgressBar_SetAttribute (TabPanel_1, TABPANEL1_PROGRESS,ATTR_PROGRESSBAR_UPDATE_MODE, VAL_PROGRESSBAR_MANUAL_MODE);
	

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
			QuitUserInterface (0);  //程序退出调用
			break;
	}
	return 0;
}
int  CVICALLBACK UnregisterServer(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			if(serverRegisted == 1)
			{
				if(ConfirmPopup("Message Confirm","Conform Unregister ?"))
				{
					serverRegisted = 0;
					UnregisterTCPServer(SERVER_PORT_CMD);
					UnregisterTCPServer(SERVER_PORT_DAT);
					SetCtrlVal(panelHandle,PANEL_TCP_LED,0);  //关闭TCP LED
					SetCtrlVal(panelHandle,PANEL_DATA_LED,0);  //关闭DATA LED
					ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,0,0);  //更新进度条  
					SetCtrlVal(TabPanel_1,TABPANEL1_PARAM_CONFIG_LED,0);
					fcnt = 0;
					paramsetted = 0;
				}
			}
			else
				MessagePopup("Warning","TCP Server not register!");
			break;
	}
	return 0;
}
int  CVICALLBACK Tcp_Config(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(serverRegisted == 1)
				MessagePopup("Message","TCP Server Registered !");
			else
			{
				if((RegisterTCPServer(SERVER_PORT_CMD,ServerCallback_CMD,NULL) < 0) || (RegisterTCPServer(SERVER_PORT_DAT,ServerCallback_DAT,NULL) < 0))
				{
					MessagePopup("Error","TCP Server Register Failed");
					serverRegisted = 0;
				}
				else
				{
					serverRegisted = 1;
					MessagePopup("Message","TCP Server Register Success,Wait Client Connection!");
				}
			}
			break;
	}
	return 0;
}
static int CVICALLBACK ServerCallback_CMD (unsigned int handle, int xType,int errCode, void *cbData)
{
	switch(xType)
	{
		case TCP_CONNECT:  //单端口，支持高并发连接
			unsigned int retcmd = 0;
			g_hconversation = handle;
			SetCtrlVal(panelHandle,PANEL_TCP_LED,1);  //成功建立连接
			unsigned int litEndian = 0xA55A040B; //字节顺序校验命令小端格式：0xA55A040B
			unsigned int bigEndian = 0x0B045AA5; //字节顺序校验命令大端格式：0x0B045AA5   
			ServerTCPWrite(g_hconversation,&litEndian,sizeof(litEndian),1000);  //小端校验 
			unsigned int state = ServerTCPRead(g_hconversation,&retcmd,sizeof(retcmd),1000);  //读取下位机返回的命令
			if(state < 0)
			{
				ServerTCPWrite(g_hconversation,&bigEndian,sizeof(bigEndian),1000);  //大端校验
				state = ServerTCPRead(g_hconversation,&retcmd,sizeof(retcmd),1000);  //读取下位机返回的命令
				if(state<0)
					MessagePopup("Error","网络通信故障，请检查网络是否断开！");
				else 
				{
					MessagePopup("Error","系统不匹配，该上位机不能使用！");
					return 0;  //直接退出
				}
			} 
			unsigned int file_check = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_FILECHECK);   
			ServerTCPWrite(g_hconversation,&file_check,sizeof(int),1000); 			
			break;
		case TCP_DISCONNECT:  //断开连接
			g_hconversation = handle;
			DisconnectTCPClient(g_hconversation);					    
			SetCtrlVal(panelHandle,PANEL_TCP_LED,0);
			MessagePopup("Message","命令端口网络连接已断开！");
			break;
		case TCP_DATAREADY:  //接收到TCP Client端发送过来的数据
			g_hconversation = handle;
			unsigned int receiveCmd = 0;
			int status = 0;
			unsigned int upload = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_UPLOAD_FILE);  //采样数据传输起始信号  
			ServerTCPRead(g_hconversation,&receiveCmd,sizeof(receiveCmd),1000); //读取命令
			unsigned char cmd = HEAD_CMD(receiveCmd); //命令 
			switch(cmd)
			{
				case CMD_GET_SWTABLE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000);
					if(status == 0)
						MessagePopup("Message","数字切换表发送成功！");
					else 
						MessagePopup("Error","数字切换表发送失败！");   
					break;
				case CMD_FILECHECK:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					switch(status)
					{
						case -1:
							MessagePopup("Message","下位机打开参数配置文件失败！");
							break;
						case -2:
							MessagePopup("Message","下位机读取参数配置文件失败！");
							break;
						case -3:
							MessagePopup("Message","下位机打开数字切换表失败！");
							break;
						case -4:
							MessagePopup("Message","下位机读取数字切换表失败！");
							break;
						case -5:
							MessagePopup("Message","下位机打开波形数据文件失败！");
							break;
						case -6:
							MessagePopup("Message","下位机读取波形数据文件失败！");
							break;
						case 0:
							MessagePopup("Message","文件完整");   
							break;
						case 1:
							MessagePopup("Message","下位机数据采集任务未完成!");    
							break;
					}
					unsigned int count = 0;
					ServerTCPRead(g_hconversation,&count,sizeof(count),1000);
					ftotal = count + 3;
					break;
				case CMD_GET_PARAMETER:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					if(status == 0)
						SetCtrlVal(TabPanel_1,TABPANEL1_PARAM_CONFIG_LED,1);
					else
						MessagePopup("Error","参数配置失败");
					break;
				case CMD_SELFCHECK:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000);
					if(status < 0)
						MessagePopup("Error","自检失败！"); //下位机打不开参数配置文件   
					else
						selfCheck();
					break;
				case CMD_GET_DAWAVE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					if(status == 0)
						MessagePopup("Message","波形数据发送成功！");
					else 
						MessagePopup("Error","波形数据发送失败！");   
					break;
				case CMD_REQUEST_FILE:
					unsigned int upload_param = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_UPLOAD_PARAMETER); 
					ServerTCPWrite(g_hconversation,&upload_param,sizeof(upload_param),1000); //发送上传参数配置文件命令 
					break;
				case CMD_UPLOAD_FILE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000);
					char filename[NAME_LEN] = {0};
					ServerTCPRead(g_hconversation,filename,sizeof(filename),1000);  //接收文件名
					char fileDir[NAME_LEN] = {0};
					strcpy(fileDir,dataDir);  //拷贝路径名
					strcat(fileDir,filename);
					if(status == 0)
					{
						saveData(fileDir); //数据回写本地
						fcnt++; //AD采样数据文件接收计数
						ServerTCPWrite(g_hconversation,&upload,sizeof(upload),1000); //发送数据上传命令
						ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,100*(double)fcnt/(double)(ftotal),0);  //更新进度条            
					}
					else if(status == -1)
					{
						saveData(fileDir); //数据回写本地
						fcnt++; //AD采样数据文件接收计数
						ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,100*(double)fcnt/(double)(ftotal),0);  //更新进度条            
						MessagePopup("Message","数据下载完成!");
					}
					else 
					{  
						MessagePopup("Error","数据下载失败");
					}
					break;
				case CMD_UPLOAD_PARAMETER:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					if(status == 0)
					{
						param = saveParam(dataDir);
						fcnt++; //AD采样数据文件接收计数
						ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,100*(double)fcnt/(double)(ftotal),0);  //更新进度条 
						unsigned int upload_tab = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_UPLOAD_SWTABLE);
						ServerTCPWrite(g_hconversation,&upload_tab,sizeof(upload_tab),1000); //发送上传数字切换表命令
					}
					else 
						MessagePopup("Error","下位机回传参数配置文件出错！");   
					break;
				case CMD_UPLOAD_SWTABLE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					if(status == 0)
					{
						saveTable(dataDir,120);  //数字切换表最多120行
						fcnt++; //AD采样数据文件接收计数
						ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,100*(double)fcnt/(double)(ftotal),0);  //更新进度条 
						unsigned int upload_wave = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_UPLOAD_DAWAVE);
						ServerTCPWrite(g_hconversation,&upload_wave,sizeof(upload_wave),1000); //发送上传波形数据命令
					}
					else 
						MessagePopup("Error","下位机回传数字切换表出错！");   
					break;					
				case CMD_UPLOAD_DAWAVE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000); 
					if(status == 0)  
					{
						saveWave(dataDir,param.mwave);  // 
						fcnt++; //AD采样数据文件接收计数
						ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,100*(double)fcnt/(double)(ftotal),0);  //更新进度条 
					}
					else 
						MessagePopup("Error","下位机回传波形数据文件出错！");  
					ServerTCPWrite(g_hconversation,&upload,sizeof(upload),1000); //发送数据上传命令  
					break;
				case CMD_DELETE_FILE:
					ServerTCPRead(g_hconversation,&status,sizeof(status),1000);
					if(status == 0)
						MessagePopup("Message","下位机删除文件成功！");
					else 
						MessagePopup("Error","下位机删除文件失败！");
					break;
				default:
					break;
			}
			break;
	}
	return 0;
}
static int CVICALLBACK ServerCallback_DAT (unsigned int handle, int xType,int errCode, void *cbData)
{
	switch(xType)
	{
		case TCP_CONNECT:  //单端口，支持高并发连接
			d_hconversation = handle;
			SetCtrlVal(panelHandle,PANEL_DATA_LED,1);  //成功建立连接
			break;
		case TCP_DISCONNECT:  //断开连接
			d_hconversation = handle;
			DisconnectTCPClient(d_hconversation);
			SetCtrlVal(panelHandle,PANEL_DATA_LED,0);
			MessagePopup("Message","数据端口网络连接已断开！");
			break;
		case TCP_DATAREADY:  //接收到TCP Client端发送过来的数据
			d_hconversation = handle;
			break;
	}
	return 0;
}
int  CVICALLBACK Table_Config(int panel, int control, int event, //加载数字切换表
							 void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			char tablepath[512] = {0};  
			int ret = FileSelectPopup("D:\\","*.txt",".","选择数字切换表",VAL_OK_BUTTON,0,1,1,0,tablepath);
			if(ret == 0)
			{
				MessagePopup("Warning","请选择文件！");
				return 0;
			}
			SetCtrlVal(TabPanel_1,TABPANEL1_SWITCH_TABLE,tablepath);
			unsigned short buffer[120][2] = {0};
			unsigned int lineNum = rdtable(tablepath,buffer);  //读取数字切换表中的内容，返回行数
			int wordsToWrite = lineNum*2;
			int wordsWrite = 0;
			while(wordsToWrite > 0)  //发送数字切换表中的数据
			{
				wordsWrite = ServerTCPWrite(d_hconversation,&buffer[lineNum*2 - wordsToWrite],lineNum*2*sizeof(short),1000)/sizeof(short);
				wordsToWrite -= wordsWrite;
			}
			unsigned int swtable = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_GET_SWTABLE);
			ServerTCPWrite(g_hconversation,&swtable,sizeof(swtable),1000);  //发送数字切换表命令
			break;
	}
	return 0;
}
int  CVICALLBACK Wave_Config(int panel, int control, int event,  //加载波形数据
							  void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			char wavepath[512] = {0}; 
			int ret = FileSelectPopup("D:\\","*.txt",".","选择数字切换表",VAL_OK_BUTTON,0,1,1,0,wavepath);
			if(ret == 0)
			{
				MessagePopup("Warning","请选择文件！");
				return 0;
			}
			if(paramsetted == 1)
			{
				SetCtrlVal(TabPanel_1,TABPANEL1_WAVEDATA,wavepath);
				unsigned short *buf = (unsigned short *)malloc(sizeof(unsigned short)*parameter.mwave);
				rdwave(wavepath,buf,parameter.mwave);
				int wordsToWrite =  parameter.mwave;
				int wordsWrite = 0;
				while(wordsToWrite > 0)  //发送波形数据
				{
					wordsWrite = ServerTCPWrite(d_hconversation,&buf[parameter.mwave - wordsToWrite],parameter.mwave*sizeof(short),1000)/sizeof(short);
					wordsToWrite -= wordsWrite;
				}
				free(buf);
				unsigned int wave = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_GET_DAWAVE); 
				ServerTCPWrite(g_hconversation,&wave,sizeof(int),1000);  //发送波形数据命令   
			}
			else
				MessagePopup("Error","请先配置参数");
			break;
	}
	return 0;
}
int  CVICALLBACK ParamFileBrowser(int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int ret = 0;
			FILE *fd = NULL;
			DirSelectPopup("D:\\","Select Directory",1,1,parampath);
			strcat(parampath,"paramset.txt");  //设定文件名
			SetCtrlVal(TabPanel_1,TABPANEL1_PARAM_DIR,parampath);
			fd = fopen(parampath,"r"); //以只读方式打开文件 
			if(fd == NULL) //打开失败
				fd = fopen(parampath,"w+");   //文件可读可写、每次都是覆盖写，文件不存在则创建
			ret = fread(&parameter,1,sizeof(parameter),fd);
			fclose(fd);
			//将参数配置文件中的内容显示到面板上
			SetCtrlVal(TabPanel_1,TABPANEL1_FCLK_DAC,parameter.fdac);
			SetCtrlVal(TabPanel_1,TABPANEL1_WAVE_LEN,parameter.mwave);
			SetCtrlVal(TabPanel_1,TABPANEL1_SAMPLE_RATE,parameter.fsample);
			SetCtrlVal(TabPanel_1,TABPANEL1_AVERAGE_NUM,parameter.navg);
			SetCtrlVal(TabPanel_1,TABPANEL1_SAMPLE_LEN,parameter.slength);
			SetCtrlVal(TabPanel_1,TABPANEL1_TRIG_MODE,parameter.trigmode);
			SetCtrlVal(TabPanel_1,TABPANEL1_SWITCH_TIME,parameter.tswitch);
			SetCtrlVal(TabPanel_1,TABPANEL1_SWITCH_TURN,parameter.kturn);
			SetCtrlVal(TabPanel_1,TABPANEL1_TURN_TIME,parameter.kinterval);
			SetCtrlVal(TabPanel_1,TABPANEL1_DELAY_TIME,parameter.delay_s); 
			SetCtrlVal(TabPanel_1,TABPANEL1_PLL1_N,parameter.pll1_n);
			SetCtrlVal(TabPanel_1,TABPANEL1_PLL2_R,parameter.pll2_r);
			SetCtrlVal(TabPanel_1,TABPANEL1_PLL2_N_PRE,parameter.pll2_n_pre);
			SetCtrlVal(TabPanel_1,TABPANEL1_PLL2_N,parameter.pll2_n);
			SetCtrlVal(TabPanel_1,TABPANEL1_DIVIDE0_1,parameter.DIVIDE0_1);
			SetCtrlVal(TabPanel_1,TABPANEL1_DIVIDE2_3,parameter.DIVIDE2_3);
			break;
	}
	return 0;
}
int  CVICALLBACK SetParameter(int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			FILE *fd = NULL;
			unsigned int paramConfig = S_HEAD_CREAT(CMD_HEAD,sizeof(int)+sizeof(parameter),CMD_GET_PARAMETER);
			GetCtrlVal(TabPanel_1,TABPANEL1_FCLK_DAC,&parameter.fdac);
			GetCtrlVal(TabPanel_1,TABPANEL1_WAVE_LEN,&parameter.mwave);
			GetCtrlVal(TabPanel_1,TABPANEL1_SAMPLE_RATE,&parameter.fsample);
			GetCtrlVal(TabPanel_1,TABPANEL1_AVERAGE_NUM,&parameter.navg);
			GetCtrlVal(TabPanel_1,TABPANEL1_SAMPLE_LEN,&parameter.slength);
			GetCtrlVal(TabPanel_1,TABPANEL1_TRIG_MODE,&parameter.trigmode);
			GetCtrlVal(TabPanel_1,TABPANEL1_SWITCH_TIME,&parameter.tswitch);
			GetCtrlVal(TabPanel_1,TABPANEL1_SWITCH_TURN,&parameter.kturn);
			GetCtrlVal(TabPanel_1,TABPANEL1_TURN_TIME,&parameter.kinterval);
			GetCtrlVal(TabPanel_1,TABPANEL1_DELAY_TIME,&parameter.delay_s);  
			GetCtrlVal(TabPanel_1,TABPANEL1_PLL1_N,&parameter.pll1_n);
			GetCtrlVal(TabPanel_1,TABPANEL1_PLL2_R,&parameter.pll2_r);
			GetCtrlVal(TabPanel_1,TABPANEL1_PLL2_N_PRE,&parameter.pll2_n_pre);
			GetCtrlVal(TabPanel_1,TABPANEL1_PLL2_N,&parameter.pll2_n);
			GetCtrlVal(TabPanel_1,TABPANEL1_DIVIDE0_1,&parameter.DIVIDE0_1);
			GetCtrlVal(TabPanel_1,TABPANEL1_DIVIDE2_3,&parameter.DIVIDE2_3);
			parameter.task_no += 1;
			fd = fopen(parampath,"w+");  //文件可读可写、每次都是覆盖写，文件不存在则创建
			fwrite(&parameter,1,sizeof(parameter),fd);
			fclose(fd);
			SetCtrlVal(TabPanel_1,TABPANEL1_PARAM_CONFIG_LED,0);  //关闭指示灯
			ServerTCPWrite(d_hconversation,&parameter,sizeof(parameter),1000);  //发送配置参数
			ServerTCPWrite(g_hconversation,&paramConfig,sizeof(paramConfig),1000);  //发送参数配置命令 
			paramsetted = 1;  //参数配置完成标志置位
			break;
	}
	return 0;
}
int  CVICALLBACK Delete_File(int panel, int control, int event, 
							void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			unsigned int deletefile = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_DELETE_FILE);     ;  
			ServerTCPWrite(g_hconversation,&deletefile,sizeof(deletefile),1000);  //删除下位机所有文件
			break;
	}
	return 0;
}
int  CVICALLBACK SelfCheck(int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			unsigned int selfcheck = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_SELFCHECK);     ;  
			ServerTCPWrite(g_hconversation,&selfcheck,sizeof(selfcheck),1000);  //发送自检命令
			break;
	}
	return 0;
}
int  CVICALLBACK SampleDataBoswer(int panel, int control, int event, 
							void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			DirSelectPopup("D:\\","Select Directory",1,1,dataDir);
			SetCtrlVal(TabPanel_1,TABPANEL1_DATA_DIR,dataDir);  
			strcat(dataDir,"\\");
			dirselected = 1;
			break;
	}
	return 0;
}
int  CVICALLBACK DownLoad(int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		case EVENT_COMMIT:
			unsigned int request_file = S_HEAD_CREAT(CMD_HEAD,sizeof(int),CMD_REQUEST_FILE);
			ProgressBar_SetPercentage(TabPanel_1,TABPANEL1_PROGRESS,0,0);  //更新进度条
			fcnt = 0;
			if(dirselected == 1)
				ServerTCPWrite(g_hconversation,&request_file,sizeof(request_file),1000);
			else
				MessagePopup("Error","请先选择文件保存路径！");
			break;
	}
	return 0;
}
