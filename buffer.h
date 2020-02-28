#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

#define NO_MORE_WORK -1

class Buffer {
public:
	Buffer() : Buffer(-1) { }
	Buffer(int max_size) : max_size(max_size) { }
	~Buffer() { }

	void push(int work) {
		unique_lock<mutex> lk(lock);
		cv.wait(lk, [this]() { return !isFull(); } );
		work_queue.push(work);
		lk.unlock();
		cv.notify_all();
	}

	void pop(int& work, int& len) {
		unique_lock<mutex> lk(lock);
		cv.wait(lk, [this]() { return !isEmpty(); } );
		work = work_queue.front();
		if (work != NO_MORE_WORK)
			work_queue.pop();
		len = work_queue.size();
		lk.unlock();
		cv.notify_all();	
	}

	/* Following method only intended to be used once, for initialization */
	void resize(int s) {
		lock.lock();
		max_size = s;
		lock.unlock();
	}

private:

	bool isFull() const {
		return work_queue.size() >= max_size;
	}

	bool isEmpty() const {
		return work_queue.size() == 0;
	}

	int max_size;
	mutex lock;
	queue<int> work_queue;
	condition_variable cv;
};

#endif
