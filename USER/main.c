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
	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(115200);	 //����1��ʼ��Ϊ9600
	USART2_Init(4800);
Usart3_Init(115200) ;
	 LCD_Init();
	 ESP8266_Init();
	while(OneNet_DevLink())			//??OneNET
		delay_ms(500);
	
	OneNet_Subscribe(topics, 1);
	 //����2��ʼ��Ϊ4800
	//printf("this is uart1\r\n");
	while(1)
	{
		if(++timeCount >= 200)									//5000/25=200   5s1��
		{
			printf( "OneNet_Publish\r\n");
			
			if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			if(len==24&&USART2_RX_BUF[1]==0x5a)//�ж������Ƿ�ΪHLW8032����
			{
				Data_Processing();//���ݴ���
			}
			USART2_RX_STA=0;//������ձ�־
		}//���ݴ���
			USART2_RX_STA=0;//������ձ�־
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(3);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
		/*if(USART2_RX_STA&0x8000)
		{					   
			len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			if(len==24&&USART2_RX_BUF[1]==0x5a)//�ж������Ƿ�ΪHLW8032����
			{
				Data_Processing();//���ݴ���
			}
			USART2_RX_STA=0;//������ձ�־
		}
		delay_ms(1000);*/
	}	 
}


void Data_Processing()
{
	char PUB_BUF[256];
	u32 VP_REG=0,V_REG=0,CP_REG=0,C_REG=0,PP_REG=0,P_REG=0,PF_COUNT=0,PF=0;
	double V=0,C=0,P=0,E_con=0;
	if(USART2_RX_BUF[0]!=0xaa)//оƬ�������������������������
	{
		unsigned char chart;
		int a;
		char strDst[256] = {0};
POINT_COLOR=RED;//��������Ϊ��ɫ
		VP_REG=USART2_RX_BUF[2]*65536+USART2_RX_BUF[3]*256+USART2_RX_BUF[4];//�����ѹ�����Ĵ���
		V_REG=USART2_RX_BUF[5]*65536+USART2_RX_BUF[6]*256+USART2_RX_BUF[7];//�����ѹ�Ĵ���
		V=(VP_REG/V_REG)*1.88;//�����ѹֵ��1.88Ϊ��ѹϵ�������������õķ�ѹ�����С��ȷ��
		printf("��ѹֵ��%0.2fV; ",V);
		a = (int)V;
		LCD_ShowxNum(140,120,V,1,16,0);
		LCD_ShowString(60,120,200,16,16,"Voltage");
		LCD_ShowString(210,120,200,16,16,"V");
		sprintf(strDst,"%d",a);
		printf("%s;",strDst);

		
		CP_REG=USART2_RX_BUF[8]*65536+USART2_RX_BUF[9]*256+USART2_RX_BUF[10];//������������Ĵ���
		C_REG=USART2_RX_BUF[11]*65536+USART2_RX_BUF[12]*256+USART2_RX_BUF[13];//��������Ĵ���
		C=((CP_REG*100)/C_REG)/100.0;//�������ֵ
		printf("����ֵ��%0.3fA; ",C);
		a = (int)C;
		LCD_ShowxNum(140,140,C,1,16,0);
		LCD_ShowString(60,140,200,16,16,"Current");
		LCD_ShowString(210,140,200,16,16,"A");
		//printf(" %X ",USART2_RX_BUF[0]);
		
	/*	if(USART2_RX_BUF[0]>0xf0)//�ж�ʵʱ�����Ƿ�δ���
		{
			printf("δ���õ��豸!");
		}
		else
		{*/
			PP_REG=USART2_RX_BUF[14]*65536+USART2_RX_BUF[15]*256+USART2_RX_BUF[16];//���㹦�ʲ����Ĵ�
			P_REG=USART2_RX_BUF[17]*65536+USART2_RX_BUF[18]*256+USART2_RX_BUF[19];//���㹦�ʼĴ���
			P=(PP_REG/P_REG)*1.88*1;//������Ч����
			printf("��Ч���ʣ�%0.2fW; ",P);
			a = (int)P;
		LCD_ShowxNum(140,160,P,1,16,0);
			LCD_ShowString(30,160,200,16,16,"Active power");
						LCD_ShowString(210,160,200,16,16,"W");
	//	}
		
		if((USART2_RX_BUF[20]&0x80)!=old_reg)//�ж����ݸ��¼Ĵ������λ��û�з�ת
		{
			k++;
			old_reg=USART2_RX_BUF[20]&0x80;
		}
		PF=(k*65536)+(USART2_RX_BUF[21]*256)+USART2_RX_BUF[22];//�������õ���������
		PF_COUNT=((100000*3600)/(PP_REG*1.88))*10000;//����1�ȵ��Ӧ����������
		E_con=((PF*10000)/PF_COUNT)/10000.0;//�������õ���
		//printf(" %d %d ",PF,PF_COUNT);
		printf("���õ�����%0.4f��\r\n",E_con);
		
	
		sprintf(PUB_BUF,"{\"V\":%0.2f,\"I\":%0.3f,\"E\":%0.4f,\"P\":%0.2f}",V,C,E_con,P);
			OneNet_Publish("v1/devices/me/telemetry", PUB_BUF);
		POINT_COLOR=RED;//��������Ϊ��ɫ
	

	}
	else//оƬ�����������ʧЧ
	{
		printf("data error\r\n");
	}
}



