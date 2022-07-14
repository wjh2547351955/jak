#include "led.h"
#include "stdio.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "usart2.h"
#include "usart3.h"
#include "esp8266.h"
#include "onenet.h"
#include "string.h"
#include "lcd.h"

void Data_Processing(void);
void Double2String(char *str,int len,double value);
void Sendstring1(char *cmd);
u8 k=0;
u16 old_reg=0;
const char *topics[] = {"v1/devices/me/telemetry"};
	 unsigned short timeCount = 0;	//??????
	 unsigned char *dataPtr = NULL;

 int main(void)
 {	
	u8 len;
	 char PUB_BUF[256];
	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(115200);	 //串口1初始化为9600
	USART2_Init(4800);
Usart3_Init(115200) ;
	 LCD_Init();
	 ESP8266_Init();
	while(OneNet_DevLink())			//??OneNET
		delay_ms(500);
	
	OneNet_Subscribe(topics, 1);
	 //串口2初始化为4800
	//printf("this is uart1\r\n");
	while(1)
	{
		if(++timeCount >= 200)									//5000/25=200   5s1次
		{
			printf( "OneNet_Publish\r\n");
			
			if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度
			if(len==24&&USART2_RX_BUF[1]==0x5a)//判断数据是否为HLW8032数据
			{
				Data_Processing();//数据处理
			}
			USART2_RX_STA=0;//清零接收标志
		}//数据处理
			USART2_RX_STA=0;//清零接收标志
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(3);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
		/*if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度
			if(len==24&&USART2_RX_BUF[1]==0x5a)//判断数据是否为HLW8032数据
			{
				Data_Processing();//数据处理
			}
			USART2_RX_STA=0;//清零接收标志
		}
		delay_ms(1000);*/
	}	 
}


void Data_Processing()
{
	char PUB_BUF[256];
	u32 VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0,PF_COUNT=0,PF=0;
	double V=0,C=0,P=0,E_con=0;
	if(USART2_RX_BUF[0]!=0xaa)//芯片误差修正功能正常，参数正常
	{
		unsigned char chart;
		int a;
		char strDst[256] = {0};
POINT_COLOR=RED;//设置字体为红色
		VP_REG=USART2_RX_BUF[2]*65536+USART2_RX_BUF[3]*256+USART2_RX_BUF[4];//计算电压参数寄存器
		V_REG=USART2_RX_BUF[5]*65536+USART2_RX_BUF[6]*256+USART2_RX_BUF[7];//计算电压寄存器
		V=(VP_REG/V_REG)*1.88;//计算电压值，1.88为电压系数，根据所采用的分压电阻大小来确定
		printf("电压值：%0.2fV; ",V);
		a = (int)V;
		LCD_ShowxNum(140,120,V,1,16,0);
		LCD_ShowString(60,120,200,16,16,"Voltage");
		LCD_ShowString(210,120,200,16,16,"V");
		sprintf(strDst,"%d",a);
		printf("%s;",strDst);

		
		CP_REG=USART2_RX_BUF[8]*65536+USART2_RX_BUF[9]*256+USART2_RX_BUF[10];//计算电流参数寄存器
		C_REG=USART2_RX_BUF[11]*65536+USART2_RX_BUF[12]*256+USART2_RX_BUF[13];//计算电流寄存器
		C=((CP_REG*100)/C_REG)/100.0;//计算电流值
		printf("电流值：%0.3fA; ",C);
		a = (int)C;
		LCD_ShowxNum(140,140,C,1,16,0);
		LCD_ShowString(60,140,200,16,16,"Current");
		LCD_ShowString(210,140,200,16,16,"A");
		//printf(" %X ",USART2_RX_BUF[0]);
		
	/*	if(USART2_RX_BUF[0]>0xf0)//判断实时功率是否未溢出
		{
			printf("未接用电设备!");
		}
		else
		{*/
			PP_REG=USART2_RX_BUF[14]*65536+USART2_RX_BUF[15]*256+USART2_RX_BUF[16];//计算功率参数寄存
			P_REG=USART2_RX_BUF[17]*65536+USART2_RX_BUF[18]*256+USART2_RX_BUF[19];//计算功率寄存器
			P=(PP_REG/P_REG)*1.88*1;//计算有效功率
			printf("有效功率：%0.2fW; ",P);
			a = (int)P;
		LCD_ShowxNum(140,160,P,1,16,0);
			LCD_ShowString(30,160,200,16,16,"Active power");
						LCD_ShowString(210,160,200,16,16,"W");
	//	}
		
		if((USART2_RX_BUF[20]&0x80)!=old_reg)//判断数据更新寄存器最高位有没有翻转
		{
			k++;
			old_reg=USART2_RX_BUF[20]&0x80;
		}
		PF=(k*65536)+(USART2_RX_BUF[21]*256)+USART2_RX_BUF[22];//计算已用电量脉冲数
		PF_COUNT=((100000*3600)/(PP_REG*1.88))*10000;//计算1度电对应的脉冲数量
		E_con=((PF*10000)/PF_COUNT)/10000.0;//计算已用电量
		//printf(" %d %d ",PF,PF_COUNT);
		printf("已用电量：%0.4f°\r\n",E_con);
		
	
		sprintf(PUB_BUF,"{\"V\":%0.2f,\"I\":%0.3f,\"E\":%0.4f,\"P\":%0.2f}",V,C,E_con,P);
			OneNet_Publish("v1/devices/me/telemetry", PUB_BUF);
		POINT_COLOR=RED;//设置字体为红色
	

	}
	else//芯片误差修正功能失效
	{
		printf("data error\r\n");
	}
}



