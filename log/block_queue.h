/*************************************************************************
    > File Name: block_queue.h
    > Author: 
    > Mail: 
    > Created Time: Fri 21 Feb 2025 07:25:22 PM CST
 ************************************************************************/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include "../common/common.h"
#include "../lock/locker.h"


//循环数组实现阻塞队列
//m_back = (m_back + 1) % m_max_size
//线程安全， 每个操作都有加锁和解锁操作

template <class T>
class block_queue {
public:
	block_queue(int max_size = 1000) {
		if (max_size <= 0) {
			exit(-1);
		}
		m_max_size = max_size;
		m_array = new T[max_size];
		m_size = 0;
		m_front = -1;
		m_back = -1;
	}

	void clear() {
		m_mutex.lock();

		m_size = 0;
		m_front = -1;
		m_back = -1;
		
		m_mutex.unlock();
	}

	~block_queue() {
		m_mutex.lock();

		if (m_array != NULL) {
			delete[] m_array;
		}

		m_mutex.unlock();
	}

	//判断队列是否满了
	bool full() {
		m_mutex.lock();

		if (m_size >= m_max_size) {
			m_mutex.unlock();
			return true;
		}

		m_mutex.unlock();
		return false;
	}

	//判断队列是否为空
	bool empty() {
		m_mutex.lock();

		if (0 == m_size) {
			m_mutex.unlock();
			return true;
		}

		m_mutex.unlock();
		return false;
	}

	//返回队首元素	参数保存队首元素
	bool front(T &value) {
		m_mutex.lock();

		if (0 == m_size) {
			m_mutex.unlock();
			return false;
		}
		value = m_array[m_front];

		m_mutex.unlock();
		return true;
	}

	//返回队尾元素
	bool back(T &value) {
		m_mutex.lock();

		if (0 == m_size) {
			m_mutex.unlock();
			return false;
		}
		value = m_array[m_back];

		m_mutex.unlock();
		return true;
	}

	//返回当前队列大小
	int size() {
		int tmp = 0;
		m_mutex.lock();

		tmp = m_size;

		m_mutex.unlock();
		return tmp;
	}

	//返回队列容量
	bool max_size() {
		int tmp = 0;
		m_mutex.lock();

		tmp = m_max_size;

		m_mutex.unlock();
		return tmp;
	}

	//往队列添加元素
	//添加元素前需要将所使用的队列的线程唤醒
	//当有元素push进队列，相当于生产者生产了一个元素
	//若当前没有线程等待条件变量，则唤醒没意义
	bool push(const T &item) {
		m_mutex.lock();

		if (m_size >= m_max_size) {
			m_cond.broadcast();
			m_mutex.unlock();
			return false;
		}
		m_back = (m_back + 1) % m_max_size;
		m_array[m_back] = item;
		m_size++;

		m_cond.broadcast();
		m_mutex.unlock();
		return true;
	}

	//弹出元素
	//若队列中没有元素，将会等待条件变量
	bool pop(T &item) {
		m_mutex.lock();

		//多个消费者需要用while判断
		while (m_size <= 0) {
			//当重新抢到互斥锁时
			//pthread_cond_wait返回0
			if (!m_cond.wait(m_mutex.get())) {
				m_mutex.unlock();
				return false;
			}
		}
		m_front = (m_front + 1) % m_max_size;
		item = m_array[m_front];
		m_size--;

		m_mutex.unlock();
		return true;
	}

	//弹出元素
	//增加超时处理
	bool pop(T &item, int ms_timeout) {
		struct timespec t = {0, 0};
		struct timeval now = {0, 0};
		gettimeofday(&now, NULL);
		m_mutex.lock();

		if (m_size <= 0) {
			t.tv_sec = now.tv_sec + ms_timeout / 1000;
			t.tv_nsec = (ms_timeout % 1000) * 1000;
			if (!m_cond.timewait(m_mutex.get(), t)) {
				m_mutex.unlock();
				return false;
			}
		}
		if (m_size <= 0) {
			m_mutex.unlock();
			return false;
		}
		m_front = (m_front + 1) % m_max_size;
		item = m_array[m_front];
		m_size--;

		m_mutex.unlock();
		return true;
	}

private:
	locker m_mutex;
	cond m_cond;

	T *m_array;
	int m_size;
	int m_max_size;
	int m_front;
	int m_back;
};

#endif
