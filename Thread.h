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

/**Enumeration representing all possible states in a thread's life cycle **/
enum State{READY, BLOCKED, RUNNING};

/**
 * A class representing a thread object.
 */
class Thread {
private:
    /**The thread's id **/
    int id;

    /**The number of quantum in which this thread occupied the CPU **/
    int numOfQuantum;

    /**A struct holding the threads environment data (including but not limited to: PC, SP) **/
    sigjmp_buf env;

    /**The priority of the thread **/
    int priority;

    /**The state of the thread**/
    State state;

    /**The stack of the thread **/
    char stack[STACK_SIZE]{};


public:
    /**
     * Constructor.
     * @param id The id for the thread to be constructed
     * @param priority The priority for the thread to be constructed
     * @param f The entry function for the thread to be started at.
     */
    Thread(const int& id, const int& priority, void (*f)(void));

    //////////////////////    setters     //////////////////////
    /**
     * Set the thread priority
     * @param newPriority The new priority to set
     */
    void setPriority(const int& newPriority){priority = newPriority;}

    /**
     * Set the thread's state
     * @param newState The new state to set
     */
    void setState(const State& newState){state = newState;}


    //////////////////////    getters     //////////////////////

    /**
     * @return The thread's quantum number
     */
    const int& getNumOfQuantum() const{return numOfQuantum;}

    /**
     * @return The thread's priority
     */
    const int& getPriority() const{return priority;}

    /**
     * @return The thread's state
     */
    const State& getState() const{return state;}

    /**
     * @return The thread's environment
     */
    sigjmp_buf& getEnv(){return env;}


    //////////////////////    methods     //////////////////////

    /**
     * Increment the thread's number of quantum
     */
    void incrementQuantum(){++numOfQuantum;}
};


#endif //EX2_THREAD_H
