/*************************************************************************
    > File Name: config.h
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 02:11:36 PM CST
 ************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "../common/common.h"
#include "../webserver/webserver.h"

using namespace std;

class Config {
public:
	Config();
	~Config() {};

	//读取参数
	void parse_arg(int argc, char* argv[]);

	//端口号
	int PORT;
	//日志写入模式
	int LOGWrite;
	//处罚组合模式
	int TRIGMode;
	//listenfd触发模式
	int LISTENTrigmode;
	//connfd触发模式
	int CONNTrigmode;
	//关闭连接
	int OPT_LINGER;
	
	//数据库连接池数量
	int sql_num;
	//线程池内线程数量
	int thread_num;
	//是否关闭日志
	int close_log;
	//并发模型选择
	int actor_model;
};


#endif
