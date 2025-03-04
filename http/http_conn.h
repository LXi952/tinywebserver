/*************************************************************************
    > File Name: http_conn.h
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 07:24:48 PM CST
 ************************************************************************/

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include "../common/common.h"
#include "../lock/locker.h"
#include "../log/log.h"
#include "../mysql/connection_pool.h"
#include "../timer/lst_timer.h"

class http_conn {
public:
	//设置读取文件名称m_real_file的大小
	static const int FILENAME_LEN = 200;
	//设置读缓冲区m_read_buf的大小
	static const int READ_BUFFER_SIZE = 2048;
	//设置写缓冲区m_write_buf的大小
	static const int WRITE_BUFFER_SIZE = 1024;

	//报文的请求方法，只用GET和POST
	enum METHOD {
		GET = 0,
		POST,
		HEAD,
		PUT,
		DELETE,
		TRACE,
		OPTIONS,
		CONNECT,
		PATH
	};
	//主状态机的状态
	enum CHECK_STATE {
		CHECK_STATE_REQUESTLINE = 0,
		CHECK_STATE_HEADER,
		CHECK_STATE_CONTENT
	};
	//报文解析的结果
	enum HTTP_CODE {
		//请求不完整，需要继续请求报文数据
		//跳转主线程继续监测读事件
		NO_REQUEST,			
		//获得完整的HTTP请求
		//调用do_request完成请求资源映射
		GET_REQUEST,		
		//HTTP请求报文有语法错误或请求资源为目录
		//跳转process_write完成响应报文
		BAD_REQUEST,		
		//请求资源不存在
		//跳转process_write完成响应报文
		NO_RESOURCE,		
		//请求资源禁止访问，没有读取权限
		//跳转process_write完成响应报文
		FORBIDDEN_REQUEST,	
		//请求资源可以正常访问
		//跳转process_write完成响应报文
		FILE_REQUEST,		
		//服务器内部错误
		INTERNAL_ERROR,		
		CLOSED_CONNECTION
	};
	//从状态机的状态
	enum LINE_STATUS {
		LINE_OK = 0,	//完整读取一行
		LINE_BAD,		//报文语法错误
		LINE_OPEN		//读取的行不完整
	};

public:
	http_conn() {}
	~http_conn() {}

public:
	//初始化
	void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
	//关闭http连接
	void close_conn(bool real_close = true);
	void process();
	//读取浏览器端发来的全部数据
	bool read_once();
	//响应报文写入函数
	bool write();
	sockaddr_in *get_address() {
		return &m_address;
	}
	//同步线程 初始化数据库 读取表
	void initmysql_result(connection_pool *connPool);
	int timer_flag;
	int improv;

private:
	void init();
	//从m_read_buf读取并处理请求报文
	HTTP_CODE process_read();
	//向m_write_buf写入响应报文数据
	bool process_write(HTTP_CODE ret);
	//从主状态机解析报文中的请求行数据
	HTTP_CODE parse_request_line(char *text);
	//从主状态机解析报文中的请求头数据
	HTTP_CODE parse_headers(char *text);
	//从主状态机解析报文中的请求内容
	HTTP_CODE parse_content(char *text);
	//生成响应报文
	HTTP_CODE do_request();
	//m_start_line已解析的字符
	//将指针偏移，指向未处理的字符
	char *get_line() {
		return m_read_buf + m_start_line;
	}
	//从状态机读取一行，分析是请求报文的哪一部分
	LINE_STATUS parse_line();
	void unmap();

	//根据响应报文格式，生成对应8个部分
	//均由do_request调用
	bool add_response(const char *format, ...);
	bool add_content(const char *content);
	bool add_status_line(int status, const char *title);
	bool add_headers(int content_length);
	bool add_content_type();
	bool add_content_length(int content_length);
	bool add_linger();
	bool add_blank_line();

public:
	static int m_epollfd;
	static int m_user_count;
	MYSQL *mysql;
	int m_state;	//读为0	写为1

private:
	int m_sockfd;
	sockaddr_in m_address;
	//存储读取的请求报文数据
	char m_read_buf[READ_BUFFER_SIZE];
	//缓冲区m_read_buf中数据的最后一个字节的下一个位置
	int m_read_idx;
	//m_read_buf读取的位置
	int m_checked_idx;
	//m_read_buf已经解析的字符个数
	int m_start_line;

	//存储发出的响应报文数据
	char m_write_buf[WRITE_BUFFER_SIZE];
	//指示buffer中的长度
	int m_write_idx;

	//主状态机的状态
	CHECK_STATE m_check_state;
	//请求方法
	METHOD m_method;

	//解析请求报文需要的变量
	//存储读取文件的名称
	char m_real_file[FILENAME_LEN];
	char *m_url;
	char *m_version;
	char *m_host;
	int m_content_length;
	bool m_linger;

	//读取服务器上的文件地址
	char *m_file_address;
	struct stat m_file_stat;
	struct iovec m_iv[2];
	int m_iv_count;
	int cgi;			//是否启用POST
	char *m_string;		//存储请求头文件
	int bytes_to_send;	//剩余发送字节数
	int bytes_have_send;//已经发送字节数
	char *doc_root;

	map<string, string> m_users;
	int m_TRIGMode;
	int m_close_log;

	char sql_user[100];
	char sql_passwd[100];
	char sql_name[100];

};

#endif
