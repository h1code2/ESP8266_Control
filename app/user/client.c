#include "cJSON.h"
#include "client.h"
#include "iconv.h"
#include "stdio.h"
#include "gpio.h"  //端口控制需要的头文件

struct espconn user_tcp_conn;

extern os_timer_t checkTimer_status1;


/**
 *统一接收
 */
void ICACHE_FLASH_ATTR status_handle(char *pdata)
{
	char* jsonObj;

	jsonObj=strstr(pdata,"{");

//	os_printf("%s", jsonObj);

//	cJSON * root=cJSON_Parse(jsonObj);
//	if(NULL!=root){
//		cJSON *result = cJSON_GetObjectItem(root, "result");
//		cJSON *author = cJSON_GetObjectItem(result, "author");
//		cJSON *origin = cJSON_GetObjectItem(result, "origin");
//		cJSON *category = cJSON_GetObjectItem(result, "category");
//		cJSON *content = cJSON_GetObjectItem(result, "content");
//
//		uart0_tx_buffer(author->valuestring, strlen(author->valuestring));
//		uart0_tx_buffer(origin->valuestring, strlen(origin->valuestring));
//		uart0_tx_buffer(category->valuestring, strlen(category->valuestring));
//		uart0_tx_buffer(content->valuestring, strlen(content->valuestring));
//		cJSON_Delete(root);
//
//	} else {
//		os_printf("\r\n error! \r\n");
//	}

	cJSON * root = cJSON_Parse(jsonObj);
		if (NULL != root) {
			cJSON *status1 = cJSON_GetObjectItem(root, "status1");
			cJSON *status2 = cJSON_GetObjectItem(root, "status2");
			cJSON *status3 = cJSON_GetObjectItem(root, "status3");
			if (cJSON_IsString(status1)&&cJSON_IsString(status2)&&cJSON_IsString(status3)) {
				char *value1 = status1->valuestring;
				char *value2 = status2->valuestring;
				char *value3 = status3->valuestring;

				if(*value1=='1'){
					 GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 0);
				}else{
					GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);
				}
				if(*value2=='1'){
					GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
				}else{
					GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);
				}
				if(*value3=='1'){
					GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 1);
				}else{
					GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);
				}
//				os_printf("string: %s\r\n", s);
//				cJSON_free((void *) s);
			}
			cJSON_Delete(root);
		} else {
			os_printf("\r\n error! \r\n");
		}
}


//成功接收到服务器返回数据函数
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg, char *pdata,
        unsigned short len) {
//    uart0_sendStr("\r\n ----- 开始接受数据----- \r\n ");
    status_handle(pdata); //
    espconn_disconnect(&user_tcp_conn);
//    uart0_sendStr("\r\n -----结束接受数据-----  \r\n ");
}

//发送数据到服务器成功的回调函数
void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg) {
//    uart0_sendStr("发送数据成功！\r\n ");
}

//断开服务器成功的回调函数
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg) {
//    uart0_sendStr("断开连接成功！\r\n ");
	os_timer_arm(&checkTimer_status1,1200,true);
}

//连接失败的回调函数，err为错误代码
void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err) {
//    uart0_sendStr("连接错误，错误代码为%d\r\n", err);
    espconn_connect((struct espconn *) arg);
}

//成功连接到服务器的回调函数
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg) {
    struct espconn *pespconn = arg;
    espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
    espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
    espconn_regist_disconcb(pespconn, user_tcp_discon_cb);

//    uart0_sendStr("\r\n ----- 请求数据开始----- \r\n");
//    uart0_tx_buffer(buffer, strlen(buffer));
//    uart0_sendStr("\r\n -----请求数据结束-----  \r\n");
    espconn_sent(pespconn, buffer, strlen(buffer));

}
void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,
    struct ip_addr *local_ip, int remote_port)
{
    //配置
    user_tcp_conn.type = ESPCONN_TCP;
    user_tcp_conn.state = ESPCONN_NONE;
    user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
    os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
    os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
    user_tcp_conn.proto.tcp->local_port = espconn_port();
    user_tcp_conn.proto.tcp->remote_port = remote_port;
    //注册
    espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);
    espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);
    //连接服务器
    espconn_connect(&user_tcp_conn);
}
