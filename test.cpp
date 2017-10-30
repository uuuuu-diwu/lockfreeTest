#include "LockFreeQueue.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>

static pthread_barrier_t barrier_start;
static pthread_barrier_t barrier_end;

int array[200];
LockFreeQueue<int,300> nq;
//std::queue<int*> nq;
void* thr1(void* arg) {
	pthread_barrier_wait(&barrier_start);
	for(int i = 0; i < 200; i += 2) {
		array[i] = 1;
		nq.push(array + i);
	}
	pthread_barrier_wait(&barrier_end);
	return (void*)0;
}

void* thr2(void* arg) {
	pthread_barrier_wait(&barrier_start);
	for(int i = 1; i < 200; i += 2) {
		array[i] = 0;
		nq.push(array + i);
	}
	pthread_barrier_wait(&barrier_end);
	return (void*)0;
}



int main()
{
	int* p;
	int zcount = 0,onecount = 0;

	assert(pthread_barrier_init(&barrier_start, NULL, 3) == 0);
    assert(pthread_barrier_init(&barrier_end, NULL, 3) == 0);
 	
    // for(int i = 0;i<200;i++)
    // 	nq.push(NULL);
    // for(int i = 0;i<200;i++)
    // 	nq.pop();
    struct timeval start,end;
	long long startusec,endusec;
	pthread_t t1;
    pthread_t t2;
    assert(pthread_create(&t1, NULL, thr1, NULL) == 0);
    assert(pthread_create(&t2, NULL, thr2, NULL) == 0);
    cpu_set_t cs;
    CPU_ZERO(&cs);
    CPU_SET(0, &cs);
    assert(pthread_setaffinity_np(t1, sizeof(cs), &cs) == 0);
    CPU_ZERO(&cs);
    CPU_SET(1, &cs);
    assert(pthread_setaffinity_np(t2, sizeof(cs), &cs) == 0);

    gettimeofday(&start,NULL);
    pthread_barrier_wait(&barrier_start);
    pthread_barrier_wait(&barrier_end);
    gettimeofday(&end,NULL);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

 	while((p = nq.pop())) {
 		printf("%d ",*p);
 		if(*p==1) onecount++;
 		if(*p==0) zcount++;
 	}
 	// while(!nq.empty()) {
 	// 	p = nq.front();
 	// 	nq.pop();
 	// 	if(*p==1) onecount++;
 	// 	if(*p==0) zcount++;
 	// }
 	printf("1 = %d 0 = %d\n",onecount,zcount);
 	printf("schedTimes = %d\n",nq.schedTimes);
 	startusec = start.tv_sec * 1000000 + start.tv_usec;
	endusec = end.tv_sec * 1000000 + end.tv_usec;
	printf("spend %lld us\n", (endusec - startusec));
 	return 0;
}
