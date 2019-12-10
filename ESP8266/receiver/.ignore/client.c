
#include "client.h"
#include "ets_sys.h"
struct espconn user_tcp_conn;



//定义数据类型
void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg){//定义发送
	os_printf("发送数据成功！");
}

/**
 * 设备长连接,若对方原因导致断线，因重新拨号。
 * 实验1: server中断连接，ESP收到消息，因重新拨号
 */
void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)//定义接收
{
	os_printf("断开连接成功！");

	//注册连接回调函数和重连回调函数
//	espconn_regist_connectcb((struct espconn *)arg,user_tcp_connect_cb);
//	espconn_regist_reconcb((struct espconn *)arg,user_tcp_recon_cb);
	os_timer_disarm(&connect_timer);// 断开连接，未连接上无需发送心跳包

	//启用连接
	espconn_connect((struct espconn *)arg);
}

/**
 * Tcp收到数据：1.喂狗
 *            2.心跳包数据
 *            3. 业务逻辑数据
 *                1）串口输出
 *
 */
void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg,
		char *pdata, unsigned short len){

//	os_printf("FUCK YOU  收到数据：%s\r\n",pdata);
	//os_printf("%d",len);
	//uart0_tx_buffer(pdata,len);
	//os_delay_us(300);//延时

	   uart0_tx_buffer(pdata,len);

//	os_printf("%s",&pdata[0]);
	// os_printf( "#### %2X %2X %X %X %X %X",(&pdata[0])[0],(&pdata[0])[1],(&pdata[0])[2],(&pdata[0])[3],(&pdata[0])[4],(&pdata[0])[5]);
//	if(strcmp(pdata,"7F7F")==0){
//		os_printf("7F7F收到数据：%s\r\n",pdata);
//	}else{
//		os_printf("F7F7收到数据：%s\r\n",pdata);
//	}
//	os_printf("F7F7收到数据：%s\r\n",pdata);
 	//uart0_sendStr(convertPData); //应该放在心跳包另外一个分之中
	checkback = false;//心跳包喂狗

}

void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err)//重新连接回调函数
{
	os_printf("user_tcp_recon_cb->连接错误，错误代码为%d\r\n",err);
	struct ip_info ipconfig;//定义结构体指针
	  wifi_get_ip_info(STATION_IF,&ipconfig);//获取本地的ip地址
	os_printf("#现在 local_ip:" IPSTR ",local_gw:" IPSTR  ",remote_netmask:" IPSTR,
				IP2STR(&ipconfig.ip), IP2STR(&ipconfig.gw),
				IP2STR(&ipconfig.netmask));
	os_timer_disarm(&connect_timer);// 未连接上无需发送心跳包

	espconn_connect((struct espconn *)arg);
}

//连接成功后，回调函数
void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)
{
	 struct espconn *pespconn=arg;
	 espconn_regist_recvcb(pespconn,user_tcp_recv_cb);//接受成功回调
	 espconn_regist_sentcb(pespconn,user_tcp_sent_cb);//发送成功 回调
	 espconn_regist_disconcb(pespconn,user_tcp_discon_cb);//重连回调

	 os_timer_disarm(&connect_timer);
	 os_timer_setfn(&connect_timer, (os_timer_func_t *)TCP_conned, pespconn);
	 os_timer_arm(&connect_timer, 30000, 1);
	 checkback = false;
	 uint8 uart_buf[128] = {0x00,0x0A,0x00,0x01,0x00,0x64,0x00,0xFA,0x00,0x32,0x00,0x02};
	 espconn_sent(pespconn,uart_buf,12);
	// espconn_sent(pespconn,"000A0001006400FA00320002",strlen("000A0001006400FA00320002"));
}

static uint8 reCallCount = 0;

/**
 * TCP 心跳包函数 ,通过应用层检测心跳包
 * 判断是否喂狗，喂狗则根据心跳周期发送心跳包
 * 若未喂狗，则reCallCount 次数后 重新连接
 */
void ICACHE_FLASH_ATTR TCP_conned(struct espconn *pespconn){
	if(checkback){
		if(reCallCount>=3){
			reCallCount =0;
			espconn_disconnect(pespconn);
			espconn_connect(pespconn);// 启动连接
		}else{
			reCallCount++;
			os_printf("失败重连，第%d次\r\n",reCallCount);
		}
	}else{// 发送心跳包
		uint8 heart_buf[6] = {0x00,0x04,0x00,0x02,0x7F,0x7F};
		espconn_sent(pespconn,heart_buf,6);
		checkback = true;
		reCallCount = 0;
	}
}

//void ICACHE_FLASH_ATTR   byteToChar(byte[] b) {
//        char c = (char) (((b[0] & 0xFF) << 8) | (b[1] & 0xFF));
//        return c;
//    }

void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port){

	//espconn参数配置 ，定义数据类型
	user_tcp_conn.type=ESPCONN_TCP;//类型tcp
	user_tcp_conn.state=ESPCONN_NONE;//状态
	user_tcp_conn.proto.tcp=(esp_tcp *)os_zalloc(sizeof(esp_tcp));//共同体分配内存空间，os_zalloc在mem.h里面
	os_memcpy(user_tcp_conn.proto.tcp->local_ip,local_ip,4);//本地ip
	os_memcpy(user_tcp_conn.proto.tcp->remote_ip,remote_ip,4);//远程ip
	user_tcp_conn.proto.tcp->local_port=espconn_port();//指定端口
	user_tcp_conn.proto.tcp->remote_port=remote_port;//远程端口

//	os_printf("原有 local_ip:" IPSTR ",local_port:" IPSTR  ",remote_ip:" IPSTR ",remote_port:" IPSTR,
//			IP2STR(user_tcp_conn.proto.tcp->local_ip), IP2STR(user_tcp_conn.proto.tcp->local_port),
//			IP2STR(user_tcp_conn.proto.tcp->remote_ip), IP2STR(user_tcp_conn.proto.tcp->remote_port));
	//espconn_set_opt(&user_tcp_conn,ESPCONN_KEEPALIVE);


	//注册连接回调函数和重连回调函数
	espconn_regist_connectcb(&user_tcp_conn,user_tcp_connect_cb);
	espconn_regist_reconcb(&user_tcp_conn,user_tcp_recon_cb);


	//启用连接
	espconn_connect(&user_tcp_conn);
}

int main(void)  /* 无参数形式 */
{

    return 0;
}
