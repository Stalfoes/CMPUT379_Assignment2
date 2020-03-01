#ifndef __BUFFER_H__
#define __BUFFER_H__

/*
	Synchronization was chosen to be seperated into a class to provide an atomic queue
	condition_variable's were used alongside mutex's to provide mutual exclusion and keep track of edge-cases
	This was chosen as a solution as it had minimal programming effort to solve all cases and was a smart implementation
*/

#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

// Used to signal the consumers in the queue that there is no more work to be consumed
#define NO_MORE_WORK -1

/* The Buffer class used for atomic queue operations between the producer and consumer */
class Buffer {
public:
	Buffer() : Buffer(-1) { }	// Equivalent to unitialized, will not be used
	Buffer(int max_size) : max_size(max_size), no_more_work_added(false) { }
	~Buffer() { }

	// Push an item to the queue. Will not push if the queue is full (2*nthreads)
	void push(int work, int& len) {	
		unique_lock<mutex> lk(lock);
		cv.wait(lk, [this]() { return !isFull(); } );	// Wait for the queue to not be full, lock
		work_queue.push(work);
		if (work == NO_MORE_WORK)						// If the work we're adding is NO_MORE_WORK...
			no_more_work_added = true;					//		keep track that we've added it
		len = length();	
		lk.unlock();									// Leave the critical section
		cv.notify_all();								// Notify the other threads the queue size has changed
	}

	// Pop an item from the queue, store it in work. After the item has been popped, store the length in len
	void pop(int& work, int& len) {
		unique_lock<mutex> lk(lock);
		cv.wait(lk, [this]() { return !isEmpty(); } );	// Wait for the queue to no longer be empty
		work = work_queue.front();						// Get the front of the queue
		if (work != NO_MORE_WORK)						// If the front isn't NO_MORE_WORK...
			work_queue.pop();							//		pop the item. We want to keep NO_MORE_WORK in the queue
		len = length();									
		lk.unlock();									// Unlock
		cv.notify_all();								// Notify the other threads the buffer has changed
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

	// Length of the queue taking into account NO_MORE_WORK
	int length() const {
		int len = work_queue.size();		
		if (no_more_work_added == true)
			len--;
		return len;
	}

	// If no more work is added, the NO_MORE_WORK is in the queue, meaning size should be realistically 1 less
	// the no_more_work_added is used to keep track of that
	bool no_more_work_added;
	int max_size;
	mutex lock;
	queue<int> work_queue;
	condition_variable cv;	// Used to control edge cases
};

#endif
