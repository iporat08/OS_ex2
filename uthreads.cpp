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

std::queue<Thread*> readyQ; //TODO
std::vector<Thread> blocked; //TODO what's the right DB?
Thread* running;
Thread* mainThread; //TODO Needed?
int numOfThreads;
std::map<int , Thread*> idMap;
int *quantumForPriority;
int totalNumOfQuantum = 0;
struct itimerval timer;

void startTimer() {
    struct sigaction sa = {0};

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &switchThread;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << "system error: sigaction error" << std::endl;
        exit(1);
    }

    int quantum = quantumForPriority[readyQ.front()->getPriority()];
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
    quantumForPriority = quantum_usecs;
    idMap = {};
    mainThread = new Thread(0, 0, nullptr);
    idMap[0] = mainThread;
    readyQ.push(mainThread);
    sigsetjmp(idMap[0]->getEnv(),1);
    numOfThreads = 1;
    startTimer();
    return 0;
}

