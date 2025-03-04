/*************************************************************************
    > File Name: threadpool.h
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 07:24:21 PM CST
 ************************************************************************/

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "../common/common.h"
#include "../lock/locker.h"
#include "../mysql/connection_pool.h"

template <typename T>
class threadpool {
public:
	//thread_number	线程池中线程数量
	//max_requests	请求队列中最多允许的、等待处理的请求的数量
	//connPool 连接池指针
	threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_requests = 10000);
	~threadpool();
	//向请求对列插入任务请求
	bool append(T *request, int state);
	bool append_p(T *request);

private:
	//工作线程运行的函数
	//不断从工作列表中取出任务并执行
	static void *worker(void *arg);
	void run();

private:
	int m_thread_number;		//线程池中线程数
	int m_max_requests;			//请求队列中允许的最大请求数
	pthread_t *m_threads;		//描述线程池的数组，大小为 m_thread_number
	std::list<T *>m_workerqueue;//请求队列
	locker m_queuelocker;		//保护请求队列的互斥锁
	sem m_queuestat;			//是否有任务需要处理
	connection_pool *m_connPool;//数据库
	int m_actor_model;			//模型切换

};

template<typename T>
threadpool<T>::threadpool(int actor_model, connection_pool *connPool, int thread_number, int max_requests) 
						 : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests),
						   m_threads(NULL), m_connPool(connPool) {
	if (thread_number <= 0 || max_requests <= 0) {
		throw std::exception();
	}

	//线程id初始化
	m_threads = new pthread_t[m_thread_number];
	if (!m_threads) {
		throw std::exception();
	}
	for (int i = 0; i < thread_number; ++i) {
		//循环创建线程，并将工作线程按要求进行运行
		if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
			delete[] m_threads;
			throw std::exception();
		}
		//将线程进行分离
		//不用单独对工作线程进行回收
		if (pthread_detach(m_threads[i])) {
			delete[] m_threads;
			throw std::exception();
		}
	}
}

template<typename T>
threadpool<T>::~threadpool() {
	delete[] m_threads;
}

//http_conn::request
template<typename T>
bool threadpool<T>::append(T *request, int state) {
	m_queuelocker.lock();

	if (m_workerqueue.size() >= m_max_requests) {
		m_queuelocker.unlock();
		return false;
	}
	request->m_state = state;
	m_workerqueue.push_back(request);

	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

template<typename T>
bool threadpool<T>::append_p(T *request) {
	m_queuelocker.lock();

	if (m_workerqueue.size() >= m_max_requests) {
		m_queuelocker.unlock();
		return false;
	}
	m_workerqueue.push_back(request);

	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

template<typename T>
void *threadpool<T>::worker(void *arg) {
	threadpool *pool = (threadpool *)arg;
	pool->run();
	return pool;
}

template<typename T>
void threadpool<T>::run() {
	while (true) {
		//信号量等待
		m_queuestat.wait();
		m_queuelocker.lock();

		if (m_workerqueue.empty()) {
			m_queuelocker.unlock();
			continue;
		}

		T *request = m_workerqueue.front();
		m_workerqueue.pop_front();

		m_queuelocker.unlock();

		if (!request) {
			continue;
		}
		if (1 == m_actor_model) {
			if (0 == request->m_state) {
				if (request->read_once()) {
					request->improv = 1;
					connectionRAII mysqlcon(&request->mysql, m_connPool);
					request->process();
				} else {
					request->improv = 1;
					request->timer_flag = 1;
				}
			} else {
				if (request->write()) {
					request->improv = 1;
				} else {
					request->improv = 1;
					request->timer_flag = 1;
				}
			}
		} else {
			connectionRAII mysqlcon(&request->mysql, m_connPool);
			request->process();
		}
	}
}

#endif
