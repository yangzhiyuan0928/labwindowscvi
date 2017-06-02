# fluctuate_sample
一个基于Labwindows cvi编写的微振动信号采集系统上位机软件，下位机是Xilinx Zynq7020，ADC选用AD9266，DAC选用LTC1668.

### 功能
* 大小端校验
* 配置下位机工作参数(数字切换表、DAC波形数据、LMK时钟、ADC采样率、其他)
* 读取每次任务数据和参数配置文件(3个)并保存
* 系统自检与工作状态检查

### 框架
* Windows事件触发机制
* TCP/IP通信
