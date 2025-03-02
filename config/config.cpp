/*************************************************************************
    > File Name: config.cpp
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 02:54:26 PM CST
 ************************************************************************/

#include "config.h"

//构造函数
Config::Config() {
	//端口号 默认9006
	PORT = 9006;
	//日志写入方式 默认同步
	LOGWrite = 0;
	//触发组合模式 默认 listenfd LT + connfd LT
	TRIGMode = 0;
	//listenfd触发模式 默认 LT
	LISTENTrigmode = 0;
	//connfd触发模式 默认 LT
	CONNTrigmode = 0;
	//关闭连接 默认不使用
	OPT_LINGER = 0;
	//数据库连接池数量 默认 8
	sql_num = 8;
	//线程池内的线程数量 默认 8
	thread_num = 8;
	//关闭日志 默认不关闭
	close_log = 0;
	//并发模型 默认是proactor
	actor_model = 0;
}

//读取命令行
void Config::parse_arg(int argc, char* argv[]) {
	int opt;
	//p 端口 l 日志读写 m 触发模式 o 关闭连接 
	//s 数据库连接池数量 t 线程池内的线程数量
	//c 关闭日志 a 并发模型
	const char *str = "p:l:m:o:s:t:c:a:";
	while ((opt = getopt(argc, argv, str)) != -1) {
		switch (opt) {
			case 'p': {
				PORT = atoi(optarg);
				break;
			}
			case 'l': {
				LOGWrite = atoi(optarg);
				break;
			}
			case 'm': {
				TRIGMode = atoi(optarg);
				break;
			}
			case 'o': {
				OPT_LINGER = atoi(optarg);
				break;
			}
			case 's': {
				sql_num = atoi(optarg);
				break;
			}
			case 't': {
				thread_num = atoi(optarg);
				break;
			}
			case 'c': {
				close_log = atoi(optarg);
				break;
			}
			case 'a': {
				actor_model = atoi(optarg);
				break;
			}
			default: {
				break;
			}
		}
	}
}




