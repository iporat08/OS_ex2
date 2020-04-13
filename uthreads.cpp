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

typedef  std::shared_ptr<Thread> smartThreadPtr;

std::deque<int> readyQ; //TODO
std::vector<smartThreadPtr> blocked; //TODO what's the right DB?
smartThreadPtr running;
int numOfThreads;
std::map<int , smartThreadPtr> idMap;
int *priorityArray;
int numOfPriorities;
int totalNumOfQuantum = 0;
struct itimerval timer;
sigset_t maskedSet;

void startTimer() {
    struct sigaction sa = {0};

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << "system error: sigaction error" << std::endl;
        exit(1);
    }

    int quantum = priorityArray[idMap[readyQ.front()]->getPriority()];
    int quantum_seconds = quantum / 1000000;
    int quantum_mseconds = quantum % 1000000;


    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = quantum_seconds;
    timer.it_value.tv_usec = quantum_mseconds;

    // configure the timer to expire every 3 sec after that... */
    timer.it_interval.tv_sec = quantum_seconds;
    timer.it_interval.tv_usec = quantum_mseconds;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
        std::cerr << "system error: setitimer error" << std::endl;
        exit(1);
    }

    switchThreads();
}


int uthread_init(int *quantum_usecs, int size){
    for(int i = 0; i < size; ++i){
        if(quantum_usecs[i] < 0){
            std::cerr << "thread library error: invalid quantum input on init" << std::endl;
            return -1;
        }
    }
    sigemptyset(&maskedSet);
    sigaddset(&maskedSet, SIGVTALRM);
    priorityArray = quantum_usecs;
    numOfPriorities = size;
    idMap = {};
    smartThreadPtr mainThread (new Thread(0, 0, nullptr));
    idMap[0] = mainThread;
    readyQ.push_back(0);
    sigsetjmp(idMap[0]->getEnv(),1);
    numOfThreads = 1;
    startTimer();
    return 0;
}

/**
 * //TODO
 * @return
 */
int getLowestIdAvailable(){
    int i;
    for(i = 1; i < MAX_THREAD_NUM; ++i){
        if(idMap.find(i) == idMap.end()){
            return i;
        }
    }
    return i;
}


int uthread_spawn(void (*f)(void), int priority){
    sigprocmask(SIG_BLOCK, &maskedSet, NULL);
    int id = getLowestIdAvailable();
    if(id == MAX_THREAD_NUM){
        std::cerr << "thread library error: number of concurrent threads exceeded the limit "
                     "(MAX_THREAD_NUM)!" << std::endl;
        sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
        return -1;
    }
    smartThreadPtr newThread(new Thread(id, priority, f));
    ++numOfThreads;
    readyQ.push_back(id);
    sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
    return id;
}

int uthread_change_priority(int tid, int priority){
    sigprocmask(SIG_BLOCK, &maskedSet, NULL);
    if(idMap.find(tid) == idMap.end() || priority >= numOfPriorities){
        std::cerr << "thread library error: tid or priority not found!" << std::endl;
        sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
        return -1;
    }
    idMap[tid]->setPriority(priority);
    sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
    return 0; //TODO or return the id of the created thread?
}


template<typename T>  // TODO: verify
void removeElement(T& container, int const& tid) {
    auto position = std::find(container.cbegin(), container.cend(), tid);
    if(position != container.cend()){
        container.erase(position);
    }
}


int uthread_terminate(int tid){
    sigprocmask(SIG_BLOCK, &maskedSet, NULL);
    if(idMap.find(tid) == idMap.end()){
        std::cerr << "thread library error: cannot terminate, tid not found!" << std::endl;
        sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
        return -1;
    }

    if(tid == 0){
        idMap.clear();
        exit(0);
    }
    if(idMap[tid] != running){
        idMap.erase(tid);
//        auto position = std::find(readyQ.cbegin(), readyQ.cend(), tid);
//        if(position != readyQ.cend()){
//            readyQ.erase(position);
//        }
        --numOfThreads;
//        auto position2 = std::find(blocked.cbegin(), blocked.cend(), tid);
//        if(position2 != blocked.cend()){
//            blocked.erase(position2);
//        }
        removeElement(readyQ, tid);
        removeElement(blocked, tid);
    }
    else {

    }

    sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
    return 0;
}


















