//
// Created by shahaf.hermann on 12/04/2020.
//

#include "Thread.h"

Thread::Thread(const int& id, const int& priority, void (*f)(void)): id(id), numOfQuantum(0),
                priority(priority), state(READY){

    if(id != 0){
        address_t sp, pc;

        sp = (address_t)stack + STACK_SIZE - sizeof(address_t);
        pc = (address_t)f;
        sigsetjmp(env, 1);
        (env->__jmpbuf)[JB_SP] = translate_address(sp);
        (env->__jmpbuf)[JB_PC] = translate_address(pc);
        sigemptyset(&env->__saved_mask); //TODO why so empty?
    }

}