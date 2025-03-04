/*************************************************************************
    > File Name: main.cpp
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 02:09:43 PM CST
 ************************************************************************/

#include "./config/config.h"
#include "./webserwer/webserwer.h"

int main(int argc, char* argv[]) {
	//数据库信息 登录名 密码 库名
	string user = "root";
	string passwd = "123";
	string databasename = "TinyWebServer";

	//命令行解析 
	Config config;
	config.parse_arg(argc, argv);

	//服务端
	WebServer server;
	//初始化
	server.init(config.PORT, user, passwd, databasename, config.LOGWrite,
				config.OPT_LINGER, config.TRIGMode, config.sql_num, config.thread_num,
				config.close_log, config.actor_model);
	//日志
	server.log_write();
	//数据库
	server.sql_pool();
	//线程池
	server.thread_pool();
	//触发模式
	server.trig_mode();
	//监听
	server.eventListen();
	//运行
	server.eventLoop();
	return 0;
}

