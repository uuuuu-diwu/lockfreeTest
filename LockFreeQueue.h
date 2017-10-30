#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__
#include <stdint.h>
#include <assert.h>
#include <sched.h>
#include <stdio.h>

#define CAS(a_ptr, a_oldVal, a_newVal) __sync_bool_compare_and_swap(a_ptr, a_oldVal, a_newVal)


template<typename T,uint32_t QSIZE>
class LockFreeQueue {
private:
	T* rawVec_[QSIZE];
	uint32_t writeIndex_;
	uint32_t readIndex_;
	uint32_t maxReadIndex_;
public:
	volatile uint32_t schedTimes;
	static inline uint32_t count2Index(uint32_t count) {
		return (count % QSIZE);
	} 

	bool push(T* data) {
		uint32_t curReadIndex;
		uint32_t curWriteIndex;
		do {
			curReadIndex = readIndex_;
			curWriteIndex = writeIndex_;
			if(count2Index(curWriteIndex + 1) == count2Index(curReadIndex)) {
				//remain one slot to different empty&full
				//now queue is full
				return false;
			}
		} while(!CAS(&writeIndex_,curWriteIndex,(curWriteIndex + 1)));
		rawVec_[count2Index(curWriteIndex)] = data;
		//we need to update maxReadIndex_
		while(!CAS(&maxReadIndex_,curWriteIndex,(curWriteIndex + 1))) {
			__sync_fetch_and_add (&this->schedTimes, 1);
			sched_yield();
			
		}
		return true;
	}

	T* pop() {
		uint32_t curReadIndex;
		uint32_t curMaxReadIndex;
		T* res;
		do {
			curReadIndex = readIndex_;
			curMaxReadIndex = maxReadIndex_;
			if(count2Index(curMaxReadIndex) == count2Index(readIndex_)) {
				return (T*)0;
			}
			res = rawVec_[count2Index(curReadIndex)];
			if(CAS(&readIndex_,curReadIndex,(curReadIndex + 1))) {
				return res;
			}
		} while(1);
		assert(0);
		return (T*)0;
	}

};
#endif