#include "../include/debug.h"
#include "hal_key.h"
#include "../include/uart.h"
#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "airkiss.h"

#include "cJSON.h"
#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "gpio.h"  //端口控制需要的头文件
#include "user_main.h"

//按键定义
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_MTMS_U
#define KEY_0_IO_NUM                            14
#define KEY_0_IO_FUNC                           FUNC_GPIO14

//连接路由器的定时器
os_timer_t checkTimer_status1;
static const char * API_URL ICACHE_RODATA_ATTR = "http://axk.huangtongx.cn/api/handle/?esp_key=yZ5mWfxmc3U6bontPQSF9Q";

LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];
LOCAL keys_typedef_t keys;

LOCAL esp_udp ssdp_udp;
LOCAL struct espconn pssdpudpconn;
LOCAL os_timer_t ssdp_time_serv;

uint8_t lan_buf[200];
uint16_t lan_buf_len;
uint8 udp_sent_cnt = 0;


void Check_Status1(void) {
    system_soft_wdt_feed();//这里我们喂下看门狗  ，不让看门狗复位
    uint8 getState;
    getState = wifi_station_get_connect_status();

    //如果状态正确，证明已经成功连接到路由器
    if (getState == STATION_GOT_IP) {
//      os_printf("WIFI连接成功！把连接路由器的定时器关闭！");
        os_timer_disarm(&checkTimer_status1);
        os_timer_disarm(&connect_timer);

        uint8 status = wifi_station_get_connect_status();
        if (status == STATION_GOT_IP) {
//            uart0_sendStr("WIFI连接成功！开始请求数据！");
            startHttpQuestByGET(API_URL);
            return;
        }
    }
}

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata) {

    switch (status) {

    //连接未开始，请勿在此阶段开始连接
    case SC_STATUS_WAIT:
        os_printf("SC_STATUS_WAIT\n");
        break;

    //发现信道
    case SC_STATUS_FIND_CHANNEL:
        os_printf("SC_STATUS_FIND_CHANNEL\n");
        break;


    //得到wifi名字和密码
    case SC_STATUS_GETTING_SSID_PSWD:
        os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
        sc_type *type = pdata;
        if (*type == SC_TYPE_ESPTOUCH) {
            os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
        } else {
            os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
        }
        break;

    case SC_STATUS_LINK:
        os_printf("SC_STATUS_LINK\n");
        struct station_config *sta_conf = pdata;

        wifi_station_set_config(sta_conf);
        wifi_station_disconnect();
        wifi_station_connect();
        break;

    //成功获取到IP，连接路由完成。
    case SC_STATUS_LINK_OVER:
        os_printf("SC_STATUS_LINK_OVER \n\n");
        if (pdata != NULL) {
            uint8 phone_ip[4] = { 0 };
            os_memcpy(phone_ip, (uint8*) pdata, 4);
            os_printf("\r\r\n\nPhone ip: %d.%d.%d.%d\n\n\r\r", phone_ip[0], phone_ip[1],  phone_ip[2], phone_ip[3]);
        }

        //停止配置
        smartconfig_stop();
        break;
    }

}

//用户自定义 RF_CAL 参数存放在 Flash 的扇区号
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    default:
        rf_cal_sec = 0;
        break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

//长按五秒按键回调
LOCAL void ICACHE_FLASH_ATTR keyLongPress(void) {

}

//短按按键回调
LOCAL void ICACHE_FLASH_ATTR keyShortPress(void) {
    os_printf("\r\r\n\n开启SmartConfig配网模式\n\n\r\r");
    smartconfig_set_type(SC_TYPE_ESPTOUCH);
    wifi_set_opmode(STATION_MODE);
    smartconfig_start(smartconfig_done);
}

//按键初始化
LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
            keyLongPress, keyShortPress);
    keys.singleKey = singleKey;
    keyParaInit(&keys);
}


void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(57600, 57600);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);//选择GPIO2
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), 1);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);//选择GPIO4
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);//选择GPIO4
    GPIO_OUTPUT_SET(GPIO_ID_PIN(5), 0);//GPIO14为高

    os_printf("\r\r\n\nSDK version:%s\n\n\r\r", system_get_sdk_version());
    os_printf("\r\r\n\nBIT_RATE_57600\n\n\r\r");
    keyInit();


//    status1
	os_timer_disarm(&checkTimer_status1); //取消定时器定时
	os_timer_setfn(&checkTimer_status1, (os_timer_func_t *) Check_Status1,
	NULL); //设置定时器回调函数s
	os_timer_arm(&checkTimer_status1, 1200, true); //启动定时器，单位：毫秒1200


}
