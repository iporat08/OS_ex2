//
// Created by shahaf.hermann on 12/04/2020.
//
#include "uthreads.h"
#include "Thread.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/time.h>
#include <memory>

#define BLOCK sigprocmask(SIG_BLOCK, &maskedSet, NULL)
#define  UNBLOCK sigprocmask(SIG_UNBLOCK, &maskedSet, NULL)
#define SPAWN_ERR "thread library error: number of concurrent threads exceeded the limit \
                                                                                (MAX_THREAD_NUM)!"
#define PRIORITY_ERR "thread library error: tid or priority not found!"
#define TERMINATE_ERR "thread library error: cannot terminate, tid not found!"
#define SIGACTION_ERR "system error: sigaction error"
#define SETITIMER_ERR "system error: setitimer error"
#define INIT_ERR "thread library error: invalid quantum input on init"
#define BLOCK_ERR "thread library error: main thread cannot be blocked or tid to block not found"
#define RESUME_ERR "thread library error: tid to resume not found"
#define BLOCKED_OR_TERMINATED -1
#define FAILURE -1
#define SUCCESS 0


typedef  std::shared_ptr<Thread> smartThreadPtr;

std::deque<int> readyQ; //TODO
//std::vector<int> blocked; //TODO what's the right DB?
int runningId;
int numOfThreads;
std::map<int , smartThreadPtr> idMap;
int *priorityArray;
int numOfPriorities;
int totalNumOfQuantum = 0;
struct itimerval timer;
sigset_t maskedSet;


void scheduler(int placeHolder){
    BLOCK;
    ++totalNumOfQuantum;
    if(runningId != BLOCKED_OR_TERMINATED){
        int ret_val = sigsetjmp(idMap[runningId]->getEnv(),1);
        printf("SWITCH: ret_val=%d\n", ret_val);//TODO delete
        if (ret_val == 1) { //TODO 1 == nonzero?
//            UNBLOCK;
            return;
        }
        readyQ.push_back(runningId);
        idMap[runningId]->setState(READY);
    }

    int quantum = priorityArray[idMap[readyQ.front()]->getPriority()];
    int quantum_seconds = quantum / 1000000;
    int quantum_mseconds = quantum % 1000000;

    timer.it_value.tv_sec = quantum_seconds;
    timer.it_value.tv_usec = quantum_mseconds;

    timer.it_interval.tv_sec = quantum_seconds;
    timer.it_interval.tv_usec = quantum_mseconds;

    runningId = readyQ.front();
    readyQ.pop_front();
    idMap[runningId]->setState(RUNNING);
    idMap[runningId]->incrementQuantum();

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        std::cerr << SETITIMER_ERR << std::endl;
        exit(1);
    }

    UNBLOCK; //TODO here?
    siglongjmp(idMap[runningId]->getEnv(),1);
}


int uthread_init(int *quantum_usecs, int size){
    for(int i = 0; i < size; ++i){
        if(quantum_usecs[i] < 0){
            std::cerr << INIT_ERR << std::endl;
            return FAILURE;
        }
    }

    struct sigaction sa = {0};

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &scheduler; //TODO scheduler?
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << SIGACTION_ERR << std::endl;
        exit(1);
    }
    sigemptyset(&maskedSet);
    sigaddset(&maskedSet, SIGVTALRM);
    priorityArray = quantum_usecs;
    numOfPriorities = size;
    idMap = {};
    uthread_spawn(nullptr, 0);
    scheduler(0);
    return SUCCESS;
}

/**
 * //TODO
 * @return
 */
int getLowestIdAvailable(){ //TODO could be replaced by a stack
    int i;
    for(i = 0; i < MAX_THREAD_NUM; ++i){
        if(idMap.find(i) == idMap.end()){
            return i;
        }
    }
    return i;
}

int uthread_spawn(void (*f)(void), int priority){
    BLOCK;
    int id = getLowestIdAvailable();
    if(id == MAX_THREAD_NUM){
        std::cerr << SPAWN_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    smartThreadPtr newThread(new Thread(id, priority, f));
    ++numOfThreads;
    readyQ.push_back(id);
    idMap[id] = newThread;
    UNBLOCK;
    return id;
}

int uthread_change_priority(int tid, int priority){
    BLOCK;
    if(idMap.find(tid) == idMap.end() || priority >= numOfPriorities){
        std::cerr << PRIORITY_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    idMap[tid]->setPriority(priority);
    UNBLOCK;
    return SUCCESS;
}

/**
 * //TODO
 */
template<typename T>  // TODO: might npt have to be template!!!
void removeElement(T& container, int const& tid) {
    auto position = std::find(container.cbegin(), container.cend(), tid);
    if(position != container.cend()){
        container.erase(position);
    }
}

int uthread_terminate(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end()){
        std::cerr << TERMINATE_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }

    if(tid == 0){
        idMap.clear();// TODO needed?
        exit(0);
    }

    idMap.erase(tid);
//        auto position = std::find(readyQ.cbegin(), readyQ.cend(), tid);
//        if(position != readyQ.cend()){
//            readyQ.erase(position);
//        }
    --numOfThreads;
    removeElement(readyQ, tid); //TODO sp points to memory space that was already freed - BUG potential

    if(tid == runningId) {
        runningId = BLOCKED_OR_TERMINATED;
        UNBLOCK;
        scheduler(0);
    }
    UNBLOCK;
    return SUCCESS;
}

int uthread_block(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end() || tid == 0){
        std::cerr << BLOCK_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    if(idMap[tid]->getState() == BLOCKED){
        UNBLOCK;
        return SUCCESS;
    }
    idMap[tid]->setState(BLOCKED);
    removeElement(readyQ, tid); // remove from READY queue if there

    if(tid == runningId){
        runningId = BLOCKED_OR_TERMINATED;
        UNBLOCK;
        scheduler(0);
    }
    UNBLOCK;
    return SUCCESS; //TODO if a running thread blocks itself what should be returned?
}

int uthread_resume(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end()){
        std::cerr << RESUME_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    if(idMap[tid]->getState() != BLOCKED){
        UNBLOCK;
        return SUCCESS;
    }
    idMap[tid]->setState(READY);
    readyQ.push_back(tid);
    UNBLOCK;
    return SUCCESS;
}

int uthread_get_tid(){
    return runningId;
}

int uthread_get_total_quantums(){
    return totalNumOfQuantum;
}

int uthread_get_quantums(int tid){
    return idMap[tid]->getNumOfQuantum();
}