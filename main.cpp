#include "uthreads.h"
#include <cstdio>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>

#define SECOND 1000000

void f(void) {
    std::cerr << "started f" << std::endl;
    int i = 0;
    while (true) {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
        }
//        usleep(SECOND);
    }
}

void f_terminating_itself(){
    std::cerr << "started f" << std::endl;
    int i = 0;
    while (uthread_get_quantums(1) == 1) {
        ++i;
        if (i % 1000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
        }
    }
//    in the second run f terminates itself and should stop running
    uthread_terminate(1);
    while(true)
    {
        ++i;
        if (i % 1000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
        }
    }
};

void g(void) {
    int i = 0;
    std::cerr << "started g" << std::endl;

    while (true) {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in g" << std::endl;
            std::cerr << "my quantum is: " << uthread_get_quantums(2) << std::endl;
        }
    }
}

void f_with_sleep(void) {
    std::cerr << "started f" << std::endl;
    while (true) {
        sleep(1);
    }
}

void g_with_sleep(void) {
    std::cerr << "started g" << std::endl;

    while (true) {
        sleep(1);
    }
}

void f_for_block(void) {
    std::cerr << "started f" << std::endl;
    int i = 0;
//    blocking g_for_block
    uthread_block(2);
//    it's an error to block the main thread
    assert(uthread_block(0) == -1);
//    blocking tid which is blocked already is fine
    assert(uthread_block(2) == 0);
    while (true) {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
        }
//        usleep(SECOND);
    }
}

void f_for_block2(void) {
    std::cerr << "started f" << std::endl;
    int i = 0;
//    blocking g_for_block
    while(uthread_get_quantums(1) == 1)
    {
        ++i;
        if (i % 10000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
        }
    }
    //        f blocks itself
    uthread_block(1);
//        this resume shouldn't work
    uthread_resume(1);
    std::cerr << "f is back to life" << std::endl;
    while(true)
    {
        ++i;
        if (i % 10000000 == 0) {
            std::cerr << "now i is " << i << " in f" << std::endl;
        }
    }
}

void g_for_block(void) {
    int i = 0;
    std::cerr << "started g" << std::endl;

    while (true) {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in g" << std::endl;
        }
//        usleep(SECOND);
    }
}

void check_init() {
    int array[3] = {-1, 2, 3};
    assert(uthread_init(array, 3) == -1);
}

void check_spawn_bad_priority() {
    int array[3] = {1, 2, 3};
    uthread_init(array, 3);
    assert(uthread_spawn(f, 3) == -1);
}

void check_spawn_limit() {
    int array[3] = {1, 2, 3};
    uthread_init(array, 3);
    int i = 0;
    while (i <= 98) {
        std::cerr << uthread_spawn(f, 2) << std::endl;
        ++i;
    }
    assert(uthread_spawn(f, 2) == -1);
}

void check_spawn_good() {
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    assert(uthread_spawn(f, 1) == 1);
    assert(uthread_spawn(g, 2) == 2);
}

void check_tid_recycle() {

}

void check_priority_change_bad() {
    int array[3] = {1, 2, 3};
    uthread_init(array, 3);
    uthread_spawn(f, 1);
    assert(uthread_change_priority(2, 2) == -1);
    assert(uthread_change_priority(1, 3) == -1);
}

void check_priority_change_good() {
//    times should change from 400000 to 200000. need to check with prints
    int array[3] = {500000, 400000, 200000};
    uthread_init(array, 3);
    uthread_spawn(f, 1);
    int i = 0;
    while (i < 1000000000) {
        ++i;
    }
    uthread_change_priority(1, 2);
}

void check_terminate1() {
//    should exit with exit code 0
    int array[3] = {500000, 400000, 200000};
    uthread_init(array, 3);
    uthread_spawn(f, 1);
    uthread_terminate(1);
}

// g is terminated imediately, then flow should go from f to main all back and forth
void check_terminate2() {
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f, 1);
    uthread_spawn(g, 2);
    uthread_terminate(2);
//    tid2 shouldn't exist now
    assert(uthread_change_priority(2, 2) == -1);
    assert(uthread_get_tid() == 0);
    while (true) {
    }
}

//checking a thread terminating itself (and it isn't the main thread)
//the run is: main, f, g, main, f (and then terminates itself), g, main, g, main...
void check_terminate3() {
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f_terminating_itself, 1);
    uthread_spawn(g, 2);
    int i = 0;
    while (true) {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in main thread" << std::endl;
        }
    }
}

// should print main thread output, then f thread, then g, and so on (indices should increase)
void print_work_flow()
{
    std::cerr << "started main thread" << std::endl;
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f, 1);
    uthread_spawn(g, 2);
    int i = 0;
    while(true)
    {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in main thread" << std::endl;
            std::cerr << "my quantum is: " << uthread_get_quantums(0) << std::endl;
        }
    }
}

void print_work_flow_with_sleep(){
    std::cerr << "started main thread" << std::endl;
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f_with_sleep, 1);
    uthread_spawn(g_with_sleep, 2);
    int i = 0;
    while(true)
    {
        ++i;
//        sleep(1);
//        std::cerr << "i in main is: " << i << std::endl;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in main thread" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(0) << std::endl;
//        }
    }
}

// f block g when it starts running. hence, only f and main should run. then, when main runs again it then resumes g and
// blocks f.
//so in total, the run is: main, f, main, g, main, g, ...
void check_block1()
{

    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f_for_block, 1);
    uthread_spawn(g_for_block, 2);
    //    it's impossible to block a thread which doesn't exist
    assert(uthread_resume(3) == -1);
//    it's impossible to block a thread which doesn't exist
    assert(uthread_block(3) == -1);
    int i = 0;
//    main started running and should have 1
    assert( uthread_get_quantums(0) == 1);
    while (uthread_get_quantums(0) == 1) {
        ++i;
//        std::cerr << "i is: " << i << std::endl;
        if (i % 10000000 == 0) {
            std::cerr << "now i is " << i << " in main" << std::endl;
        }
    }
    //        resuming g and blocking f
    uthread_resume(2);
//    it's ok to resume thread which is already resumed
    assert(uthread_resume(2) == 0);
//    it's ok to resume the running thread and has no effect
    assert(uthread_resume(0) == 0);
    uthread_block(1);
    while(true)
    {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in main" << std::endl;
        }
    }
}

// the run is: main, f, g, main, f (and then it blocks itself), g, main (here it resumes f), g, f, main, g, f...
void check_block2()
{
    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
    uthread_init(array, 3);
    uthread_spawn(f_for_block2, 1);
    uthread_spawn(g_for_block, 2);
    int i = 0;
    while(uthread_get_quantums(0) < 3)
    {
        ++i;
        if (i % 1000000 == 0) {
            std::cerr << "now i is " << i << " in main" << std::endl;
        }
    }
    std::cerr << "main is now bringing f back to life" << std::endl;
    uthread_resume(1);
    while(true)
    {
        ++i;
        if (i % 100000000 == 0) {
            std::cerr << "now i is " << i << " in main" << std::endl;
        }
    }
}

int main() {
//    check_init();
//    check_spawn_bad_priority();
    check_spawn_good();
//    check_spawn_limit();
//    check_priority_change_bad();
//    check_priority_change_good();
//    check_terminate1();
//    check_terminate2();
//    check_terminate3();
//check_tid_recycle();
//    print_work_flow();
//    check_block1();
//    check_block2();
//    print_work_flow_with_sleep(); //TODO waiting for checking sleep



    while (true) {
    }
    return 0;
}