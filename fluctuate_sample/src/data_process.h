#ifndef __DATA_PROCESS_H_
#define __DATA_PROCESS_H_

#define SERVER_PORT_CMD     60400   //command port
#define SERVER_PORT_DAT     60500   //data port

#define NAME_LEN 			512

#define HEAD_TAG(x)  ((x >>16) & 0xffff)
#define HEAD_CMD(x)  (x & 0xff)
#define HEAD_LEN(x)  ((x >>8) & 0xff)
#define CMD_HEAD     0xa55a
#define S_HEAD_CREAT(CMD_HEAD,CMD_LEN,CMD) ((CMD_HEAD & 0xffff) <<16) | ((CMD_LEN & 0xff) << 8) | ( CMD & 0xff)

//CMD: 
#define CMD_UPLOAD_PARAMETER	0x01 
#define CMD_UPLOAD_DAWAVE		0x02 
#define CMD_UPLOAD_SWTABLE		0x03 
#define CMD_UPLOAD_FILE			0x04 
#define CMD_GET_PARAMETER   	0x05 
#define CMD_GET_DAWAVE			0x06
#define CMD_GET_SWTABLE			0x07
#define CMD_SELFCHECK			0x08
#define CMD_FILECHECK   		0x09 
#define CMD_REQUEST_FILE		0x0A 
#define CMD_SYSENDIAN   		0x0B
#define CMD_DELETE_FILE			0x0C

typedef struct
{
	unsigned int task_no;   //任务号
	unsigned int fdac;      //DA时钟频率
	unsigned int mwave;     //DA波形长度
	unsigned int fsample;   //AD采样率
	unsigned int navg;      //AD采集平均次数 
	unsigned int slength;   //AD采样长度
	unsigned int tswitch;   //切换时间间隔 
	unsigned int kturn;     //切换轮次
	unsigned int kinterval; //轮次时间间隔   
	unsigned int delay_s;   //数字IO输出延时时间间隔
	unsigned int trigmode;  //采样模式
	unsigned int pll1_n;
	unsigned int pll2_r;
	unsigned int pll2_n_pre;
	unsigned int pll2_n;
	unsigned int DIVIDE0_1;
	unsigned int DIVIDE2_3;
} paramSet;

extern paramSet parameter;  //面板参数结构体 
extern paramSet param; //下位机回发的参数

extern int g_hconversation;  //TCP Cmd handle
extern int d_hconversation;  //TCP Data handle

void rdwave(char filename[],unsigned short buffer[], int len);  //读取波形数据文件中前len行数据
int rdtable(char filename[],unsigned short buffer[][2]);  //将数字切换表中的数据读取到buffer中,返回切换表中数据行数
void saveData(char filename[]);  //
void selfCheck();
paramSet saveParam(char *directory);
void saveTable(char *directory,int len);
void saveWave(char *directory,int len);

#endif
