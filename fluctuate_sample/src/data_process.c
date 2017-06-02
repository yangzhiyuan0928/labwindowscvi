#include <ansi_c.h>
#include <tcpsupp.h>
#include <userint.h>
#include <stdio.h>
#include <stdlib.h>
#include "data_process.h"
#include <string.h> 


void rdwave(char filename[],unsigned short buffer[], int len)  //read len rows wave data from wave file
{
	FILE *fp = NULL;
	if((fp = fopen(filename,"r")) == NULL)
	{
		printf("波形数据文件打开失败!\n");
		return ;
	}
	char buf[20] = {0};
	for(int i=0; i<len; i++)
	{
		fgets(buf,sizeof(buf),fp);
		unsigned short temp = (unsigned short)(atof(buf)*1000+5000);  
		buffer[i] = (unsigned short)(temp*65535.0/10000);
	}
	fclose(fp);
}
int rdtable(char filename[],unsigned short buffer[][2])  //read data from switchTable file
{
	FILE *fp = NULL;
	if((fp = fopen(filename,"r")) < 0)
	{
		printf("数字切换表打开失败!\n");
		return 0;
	}
	char buf[20] = {0};
	int lineNum = 0;
	char val1[20] = {0}, val2[20] = {0};
	while(!feof(fp))
	{
		fgets(buf,sizeof(buf),fp);
		for(int i=0; i<20; i++)
		{
			if(buf[i] == '\t')
			{
				strncpy(val1,buf,i);
				strncpy(val2,&buf[i+1],20-i-1);
			}
		}
		buffer[lineNum][0] = atoi(val1);
		buffer[lineNum][1] = atoi(val2);
		lineNum++;
		if(feof(fp))  //文件结束
			break;
	}
	fclose(fp);
	return lineNum;
}
void saveData(char *fileDir)  //save ADC sample data
{
	int messageSize = param.slength;
	int wordsToRead = messageSize;
	int wordsRead = 0;
	short *buf = (short *)malloc(sizeof(short)*parameter.slength*2); 
	while(wordsToRead > 0)   
	{
		wordsRead = ServerTCPRead(d_hconversation,&buf[messageSize - wordsToRead],messageSize*sizeof(short),5000)/sizeof(short);
		wordsToRead -= wordsRead;
	}
	FILE *fp = NULL;
	fp = fopen(fileDir,"w+");
	if(fp == NULL)
	{
		MessagePopup("Error","文件创建失败！");
		return ;
	}
	for(int i=0;i<parameter.slength;i++)
	{
		float temp = (float)buf[i]/32767*5*(-1);  //reverse
		char ret[20] = {0};
		sprintf(ret,"%.3f\n",temp); 
		fputs(ret,fp);
	}
	fclose(fp);
	free(buf);
}
void selfCheck() // System check self
{
	int array[18] = {0};  //18个数据
	int *ptr = (int *)&parameter;
	ServerTCPRead(d_hconversation,array,sizeof(array),5000); //接收参数
	if(array[0] != 0)
	{
		MessagePopup("Error","自检失败！");
		return;
	}
	for(int i=0;i<sizeof(parameter)/sizeof(unsigned int);i++)
	{
		if(array[i+1] != ptr[i])
		{
	   		MessagePopup("Error","自检失败！");
			return;
		}
	}
	MessagePopup("Message","自检成功");
}
paramSet saveParam(char *directory)
{
	paramSet param;
	ServerTCPRead(d_hconversation,&param,sizeof(param),5000);
	int *ptr = (int *)&param;
	char fileDir[NAME_LEN] = {0};
	strcpy(fileDir,directory);  //拷贝路径名
	strcat(fileDir,"parameter.txt");
	FILE *fp = NULL;
	fp = fopen(fileDir,"w+");  //写界面上参数配置文件
	if(fp == NULL)
	{
		MessagePopup("Error","参数配置文件创建失败！");
		return ;
	}
	for(int i=0;i<sizeof(param)/sizeof(unsigned int);i++)
	{
		char ret[20] = {0};
		sprintf(ret,"%d\n",ptr[i]);  //转换为字符串
		fputs(ret,fp);
	}
	fclose(fp);  //将界面参数更新到参数配置文件
	return param;
}	
void saveTable(char *directory,int len) 
{
	unsigned short *buf = (unsigned short *)malloc(sizeof(unsigned short)*len*2);  //2维数组
	memset(buf,0,len*2);
	int size = ServerTCPRead(d_hconversation,buf,sizeof(unsigned short)*len*2,5000)/sizeof(short);  //返回接收数字切换表中实际数据个数
	char fileDir[NAME_LEN] = {0};
	strcpy(fileDir,directory);  //拷贝路径名
	strcat(fileDir,"SwitchTable.txt");
	FILE *fp = NULL;
	fp = fopen(fileDir,"w+");
	if(fp == NULL)
	{
		MessagePopup("Error","数字切换表创建失败！");
		return ;
	}
	for(int i=0;i<size/2;i++)
	{
		char ret[20] = {0};
		sprintf(ret,"%d %d\n",buf[2*i],buf[2*i+1]);
		fputs(ret,fp);
	}
	free(buf);
	fclose(fp);
}
void saveWave(char *directory,int len)
{	
	unsigned short *buf = (unsigned short *)malloc(sizeof(unsigned short)*len*2);  //波形数据
	memset(buf,0,sizeof(unsigned short)*len*2);
	int messageSize = len; //messageSize根据前面参数配置时设置的采样参数计算得到
	int wordsToRead = messageSize;
	int wordsRead = 0;
	while(wordsToRead > 0) 
	{
		wordsRead = ServerTCPRead(d_hconversation,&buf[messageSize - wordsToRead],messageSize*sizeof(short),5000)/sizeof(short);
		wordsToRead -= wordsRead;
	} 
	char fileDir[NAME_LEN] = {0};
	strcpy(fileDir,directory);  //拷贝路径名
	strcat(fileDir,"wave.txt");
	FILE *fp = NULL;
	fp = fopen(fileDir,"w+");
	if(fp == NULL)
	{
		MessagePopup("Error","波形数据文件创建失败！");
		return ;
	}
	for(int i=0;i<len;i++)
	{
		char ret[20] = {0};
		double temp = (buf[i]*10000.0/65535 - 5000)/1000; 
		sprintf(ret,"%.3f\n",temp);
		fputs(ret,fp);
	}
	free(buf); 
	fclose(fp);
}
