/*
 * http.h
 *
 *  Created on: 2018��9��22��
 *      Author: taomingshuang
 */

#ifndef APP_USER_SENIVERSE_H_
#define APP_USER_SENIVERSE_H_

#include"c_types.h"
#include"uart.h"
#include"string.h"
#include"espconn.h"
#include"ip_addr.h"
#include"mem.h"
#include "user_interface.h"
#include"osapi.h"
#include"ssd1306.h"


#define GET "GET /%s HTTP/1.1\r\nContent-Type: text/html;charset=utf-8\r\nAccept: */*\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n"

struct espconn user_tcp_conn;
struct ip_addr addr;
bool ch=1;
//bool data_ok=0;
//bool refresh=0;
//bool hw_time=0;
char Data_update_OK=0;
//char *udata;
//char userdat=0;
char host[32];
char filename[208];
char buffer[1024];
unsigned short port;
struct	Weather_data{
	uint8 city[20];
	uint8 text[10];
	uint8 code[4];
	uint8 temp[5];
	uint8 updata[30];
};
struct	times{
	uint8 time[9];
	uint8 date[11];
	uint8 week[2];
	uint8 temp[5];
};

const char *code[]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","99"};

struct	times times_data;
struct	Weather_data w_data;
static os_timer_t os_timer_1;
static os_timer_t os_timer_2;
static os_timer_t os_timer_3;

void ICACHE_FLASH_ATTR http_parse_request_url(char *URL,char *host,char *filename,unsigned short *port)
{
	char *PA;
	char *PB;
	os_memset(host,0,sizeof(host));			//�������
	os_memset(filename,0,sizeof(filename));	//�������
	*port = 0;

	if (!(*URL))return;
	PA = URL;
	if (!strncmp(PA, "http://", os_strlen("http://")))
	{
		PA = URL + os_strlen("http://");
	}
	else if (!strncmp(PA, "https://",os_strlen("https://")))
	{
	    PA = URL + os_strlen("https://");
	}
	PB = strchr(PA, '/');
	if (PB)
	{
		os_memcpy(host, PA, os_strlen(PA) - os_strlen(PB));

	    if (PB + 1)
	    {
	    	os_memcpy(filename, PB + 1, os_strlen(PB - 1));
	        filename[os_strlen(PB) - 1] = 0;
	        os_printf("\r\n----- filename-----\r\n ");
	        uart0_tx_buffer(filename, strlen(filename));
	    }
	    host[os_strlen(PA) - os_strlen(PB)] = 0;
	    os_printf("\r\n----- host-----\r\n ");
	    uart0_tx_buffer(host, strlen(host));
	}
	else
	{
	    os_memcpy(host, PA, os_strlen(PA));
	    host[os_strlen(PA)] = 0;
	}
	PA = strchr(host, ':');
	if (PA)
		*port = atoi(PA + 1);
	else
	    *port = 80;
}
void ICACHE_FLASH_ATTR times_data_refresh(char *pdata)
{
	char *a;
	char *b;

	a=strstr(pdata,"datetime_1");  //����test ��λ��
	if(a)
	{
		b=strchr(a,':');
		//os_printf("���� b :  ��λ��    %s\r\n",b);
		a=strchr(b,' ');
		//os_printf("���� a �ո�  ��λ��    %s\r\n",a);
		os_memcpy(times_data.date,b+2,os_strlen(b+2)-os_strlen(a));
		times_data.date[os_strlen(b+2)-os_strlen(a)]='\0';
		b=strchr(a,',');
		os_memcpy(times_data.time,a+1,os_strlen(a+1)-os_strlen(b-1));
		times_data.time[os_strlen(a+1)-os_strlen(b-1)]='\0';
		//os_printf("���� b ,  ��λ��    %s\r\n",a);
		//OLED_P8x16Str(0,0,times_data.datatime);
	}
	a=strstr(pdata,"week_1");  //����test ��λ��
	if(a)
	{
		b=strchr(a,':');
		//os_printf("���� b :  ��λ��    %s\r\n",b);
		a=strchr(b,',');
		//os_printf("���� a ,  ��λ��    %s\r\n",a);
		os_memcpy(times_data.week,b+2,os_strlen(b+2)-os_strlen(a-1));
		times_data.week[os_strlen(b+2)-os_strlen(a-1)]='\0';
		//OLED_P8x16Str(0,2,times_data.week);
	}
	os_printf("\r\n    ��ǰ����        %s\r\n",times_data.date);
	os_printf("\r\n    ��ǰʱ��        %s\r\n",times_data.time);
	os_printf("\r\n    ����        %s\r\n",times_data.week);
}
void ICACHE_FLASH_ATTR weather_data_refresh(char *pdata)
{
	char *a;
	char *b;

	a=strstr(pdata,"name");  //����test ��λ��
	if(a)
	{
		b=strchr(a,':');
	    //os_printf("���� b :  ��λ��    %s\r\n",b);
	    a=strchr(b,',');
	    //os_printf("���� a ,  ��λ��    %s\r\n",a);
	    	    	//aa=os_strlen(b+2)-os_strlen(a-1);
	    os_memcpy(w_data.city,b+2,os_strlen(b+2)-os_strlen(a-1));
	    w_data.city[os_strlen(b+2)-os_strlen(a-1)]='\0';
	    //OLED_P8x16Str(0,0,w_data.city);
	}
	a=strstr(pdata,"text");  //����test ��λ��
	if(a)
	{
	    b=strchr(a,':');
	    //os_printf("���� b :  ��λ��    %s\r\n",b);
	    a=strchr(b,',');
	    //os_printf("���� a ,  ��λ��    %s\r\n",a);
	       	    	//aa=os_strlen(b+2)-os_strlen(a-1);
	    os_memcpy(w_data.text,b+2,os_strlen(b+2)-os_strlen(a-1));
	    w_data.text[os_strlen(b+2)-os_strlen(a-1)]='\0';
	       	//OLED_P8x16Str(0,2,w_data.text);
	}
	a=strstr(pdata,"code");  //����test ��λ��
	if(a)
	{
		b=strchr(a,':');
		//os_printf("���� b :  ��λ��    %s\r\n",b);
		a=strchr(b,',');
		//os_printf("���� a ,  ��λ��    %s\r\n",a);
		       	    	//aa=os_strlen(b+2)-os_strlen(a-1);
		os_memcpy(w_data.code,b+2,os_strlen(b+2)-os_strlen(a-1));
		w_data.code[os_strlen(b+2)-os_strlen(a-1)]='\0';
		       	//OLED_P8x16Str(0,2,w_data.text);
	}
	a=strstr(pdata,"temperature");  //����test ��λ��
	if(a)
	{
	    b=strchr(a,':');
	    //os_printf("���� b :  ��λ��    %s\r\n",b);
	    a=strchr(b,',');
	    //os_printf("���� a ,  ��λ��    %s\r\n",a);
	           	    	//aa=os_strlen(b+2)-os_strlen(a-1);
	    os_memcpy(w_data.temp,b+2,2);
	    w_data.temp[2]='\0';
	        //OLED_P8x16Str(0,4,w_data.temp);
	}
	a=strstr(pdata,"last_update");  //����test ��λ��
	if(a)
	{
		b=strchr(a,':');
		//os_printf("���� b :  ��λ��    %s\r\n",b);

		           	    	//aa=os_strlen(b+2)-os_strlen(a-1);
		os_memcpy(w_data.updata,b+2,25);
		w_data.updata[25]='\0';
		        //OLED_P8x16Str(0,4,w_data.temp);
	}
	os_printf("\r\n    ����        %s\r\n",w_data.city);
	os_printf("\r\n    ����        %s\r\n",w_data.text);
	os_printf("\r\n    �¶�        %s\r\n",w_data.temp);
	os_printf("\r\n    ʱ��        %s\r\n",w_data.updata);
}

//�ɹ����յ��������������ݺ���
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg, char *pdata, unsigned short len)
{

	os_printf("\r\n----- ��ʼ��������-----\r\n ");
    uart0_tx_buffer(pdata, strlen(pdata));
    os_printf("\r\n-----������������-----\r\n ");
    if(ch)
    {
    	OLED_P8x16Str(0,2,"times updat OK");
    	times_data_refresh(pdata);
    	espconn_disconnect(&user_tcp_conn);
    	Data_update_OK=1;
    }
    else
    {

    	OLED_P8x16Str(0,2,"weather updat OK");
    	weather_data_refresh(pdata);
    	espconn_disconnect(&user_tcp_conn);
    	Data_update_OK=2;
    }

}

//�������ݵ��������ɹ��Ļص�����
void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg) {
	os_printf("�������ݳɹ���\r\n ");
}

//�Ͽ��������ɹ��Ļص�����
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg) {
	os_printf("�Ͽ����ӳɹ���\r\n ");
	os_timer_arm(&os_timer_2,1000,0);
}



//����ʧ�ܵĻص�������errΪ�������
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err)
{
	os_printf("���Ӵ��󣬴������Ϊ%d\r\n", err);
    espconn_connect((struct espconn *) arg);
}


//�ɹ����ӵ��������Ļص�����
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;
    espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
    espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
    espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

    os_printf("\r\n ----- �������ݿ�ʼ----- \r\n");
    uart0_tx_buffer(buffer, strlen(buffer));
    os_printf("\r\n -----�������ݽ���-----  \r\n");

    espconn_sent(pespconn, buffer, strlen(buffer));

}
void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip, int remote_port)
{
    //����

    user_tcp_conn.type = ESPCONN_TCP;
    user_tcp_conn.state = ESPCONN_NONE;
    user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
    os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
    os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
    user_tcp_conn.proto.tcp->local_port = espconn_port();
    user_tcp_conn.proto.tcp->remote_port = remote_port;
    //ע��
    espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
    espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);


    espconn_set_opt(&user_tcp_conn,ESPCONN_START);
    //���ӷ�����
   espconn_connect(&user_tcp_conn);

}



void ICACHE_FLASH_ATTR user_esp_dns_found(const char *name, ip_addr_t *ipaddr,void *arg )
{
	struct ip_info info;
	wifi_get_ip_info(STATION_IF, &info);
	my_station_init(ipaddr, &info.ip, port);
}


#endif /* APP_USER_SENIVERSE_H_ */
