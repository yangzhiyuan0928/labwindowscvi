# TDLAS_TCP_SERVER
一个基于Labwindows CVI平台编写的TDLAS气体检测系统数据采集上位机软件，下位机是Xilinx Zynq7020开发板，ADC选用AD7989，DAC选用LTC4624.

## 功能
* DDS参数配置
* Laser电流、温度设置与监测
* AD数据采集控制

## 框架
* Windows事件触发机制
* TCP socket通信
* Semphore用于线程同步

## 改善
* 与下位机通信时，数据类型匹配
* 结束AD采样时，程序会卡顿
