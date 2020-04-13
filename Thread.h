//
// Created by shahaf.hermann on 12/04/2020.
//
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "uthreads.h"


#ifndef EX2_THREAD_H
#define EX2_THREAD_H


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

enum State{READY, BLOCKED, RUNNING};

class Thread {
private:
    int id;
    int numOfQuantum;
    sigjmp_buf env {}; //TODO does this initialization work?
    int priority;
    State state; //TODO needed?
    char stack[STACK_SIZE]{}; //TODO is {} needed?


public:
    Thread(const int& id, const int& priority, void (*f)(void));

    //////////////////////    setters     //////////////////////
    void setPriority(const int& newPriority){priority = newPriority;}

    void setState(const State& newState){state = newState;}


    //////////////////////    getters     //////////////////////

    const int& getId(){return id;}

    const int& getNumOfQuantum(){return numOfQuantum;}

    const int& getPriority(){return priority;} //TODO needed?

    const State& getState(){return state;}

    sigjmp_buf& getEnv(){return env;} //TODO maybe the Thread should handle env changes?


    //////////////////////    methods     //////////////////////

    void incrementQuantum(){++numOfQuantum;} //TODO private or public?
};


#endif //EX2_THREAD_H
