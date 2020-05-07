//
// Created by eliasital
//

/**
 * boolean print (global var) :
 *     prints to screen if true, otherwise runs, but without any prints.
 *
 * printCount (function local variable) :
 *     Functions runs and writes 'in f(0)', 'in f(1)' and so on.
 *     The number in the brackets is the printCount var.
 *     A condition of 'if (printCount == x) then ...'
 *     states what will the function do when it will reach its 'x' cycle.
 *
 * Tests :
 *     1    Spawn functions
 *     2    Non-Main function terminates it self
 *     3    Non-Main function blocks Non-Main function
 *     4    Non-Main function goes to sleep
 *     5    Main terminates functions
 *     6    Main blocks function
 *     7    Main unblocks function
 *     8    Main removes blocked function
 *     9    Main terminates with functions that are alive
 *     10   Sanity check - Main blocks sleeping function (OK!)
 *          Sanity check - Main unblocks sleeping function (ERR!)
 *          Sanity check - Main blocks non-existent function (ERR!)
 *          Sanity check - Main asks to sleep (ERR!)
 *          Sanity check - Main asks to block himself (ERR!)
 *          Sanity check - Main asks to unblock sleeping function (ERR!)
 */

#include <array>
#include <iostream>
#include "uthreads.h"

#define QUANT 1000000
#define PRINTER 100000000

bool print = true;

using namespace std;

/**
 * A Simple function - runs and terminates itself in the end.
 */
void f()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in f(" << printCount << ")" << endl;}
            if (printCount == 7)
            {
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "f terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}

/**
 * A Simple function - runs and terminates itself in the end.
 */
void g()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in g(" << printCount << ")" << endl;}
            if (printCount == 13)
            {
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "g terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_h()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in h(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_i()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in i(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

/**
 * A Simple function - runs forever, until terminated.
 */
void neverEnding_j()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in j(" << printCount << ")" << endl;}
            printCount++;
        }
    }
}

///**
// * A simple function that sends it self to sleep.
// */
//void sleeper()
//{
//    int printCount = 0;
//    for (int i = 0; ; i++)
//    {
//        if (i%PRINTER == 0)
//        {
//            if (print) {cout << "in sleeper(" << printCount << ")" << endl;}
//
//            if (printCount == 15)
//            {
//                if (print) {cout << "sleeper went to sleep for 50 quants!" << endl;}
//                uthread_sleep(50);
//                if (print) {cout << "sleeper is awake again!" << endl;}
//            }
//
//            if (printCount == 57)
//            {
//                break;
//            }
//            printCount++;
//        }
//    }
//
//    if (print) {cout << "sleeper terminates" << endl;}
//    int me = uthread_get_tid();
//    uthread_terminate(me);
//}

/**
 * Blocks some thread and then unblocks its.
 * Blocks it self.
 */
void blocker()
{
    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "in blocker(" << printCount << ")" << endl;}

            if (printCount == 3)
            {
                if (print) {cout << "blocker blocks f (tid = 1)!" << endl;}
                uthread_block(1);
            }

            if (printCount == 11)
            {
                if (print) {cout << "blocker unblocks f (tid = 1)!" << endl;}
                uthread_resume(1);
            }

            if (printCount == 40)
            {
                if (print) {cout << "blocker blocks himself!" << endl;}
                uthread_block(uthread_get_tid());
                if (print) {cout << "blocker was resumed!" << endl;}
                break;
            }
            printCount++;
        }
    }
    if (print) {cout << "blocker terminates" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}


int main()
{

    int q[2] = {10, 20};
    int init = uthread_init(q, 2);

    if (print) {cout << "MAIN INIT : " << init << endl << endl;}

    int fid = uthread_spawn(&f, 0);
    if (print) {cout << "MAIN SPAWNS f : " << fid << endl;}

    int gid = uthread_spawn(&g, 1);
    if (print) {cout << "MAIN SPAWNS g : " << gid << endl;}

    int hid = uthread_spawn(&neverEnding_h, 0);
    if (print) {cout << "MAIN SPAWNS h : " << hid << endl;}

    int iid = uthread_spawn(&neverEnding_i, 1);
    if (print) {cout << "MAIN SPAWNS i : " << iid << endl;}

    int jid = uthread_spawn(&neverEnding_j, 0);
    if (print) {cout << "MAIN SPAWNS j : " << jid << endl;}

//    int sleeperid = uthread_spawn(&sleeper, 1);
//    if (print) {cout << "MAIN SPAWNS sleeper : " << sleeperid << endl;}

    int blockerid = uthread_spawn(&blocker, 0);
    if (print) {cout << "MAIN SPAWNS blockerid : " << blockerid << endl;}

    if (print) {cout << endl << "MAIN RUNS!" << endl;}

    int printCount = 0;
    for (int i = 0; ; i++)
    {
        if (i%PRINTER == 0)
        {
            if (print) {cout << "IN MAIN(" << printCount << ")" << endl;}

            if (printCount == 20)
            {
//                if (print) {cout << "GOOD-ERR Main blocks sleeping sleeper!" << endl;}
//                uthread_block(sleeperid);
//                if (print) {cout << "ERR Main unblocks sleeping sleeper!" << endl;}
//                uthread_resume(sleeperid);
                if (print) {cout << "ERR Main blocks non-existent 99!" << endl;}
                uthread_block(99);
                if (print) {cout << "ERR Main unblocks non-existent 101!" << endl;}
                uthread_block(101);
                if (print) {cout << "ERR Main asks to block himself!" << endl;}
                uthread_block(uthread_get_tid());
            }

            if (printCount == 30)
            {
                if (print) {cout << "Main removes h,i!" << endl;}
                uthread_terminate(hid);
                uthread_terminate(iid);

//                if (print) {cout << "ERR Main unblocks sleeping sleeper!" << endl;}
//                uthread_resume(sleeperid);
            }


            if (printCount == 45)
            {
                if (print) {cout << "Main unblocks blocker (which blocked himself)!" << endl;}
                uthread_resume(blockerid);
            }

            if (printCount == 52)
            {
                if (print) {cout << "Main blocks blocker!" << endl;}
                uthread_block(blockerid);
            }

            if (printCount == 58)
            {
                if (print) {cout << "Main removes blocker!" << endl;}
                uthread_terminate(blockerid);
            }

            if (printCount == 65)
            {
                break;
            }
            printCount++;
        }
    }

    if (print) {cout << "MAIN TERMINATES WILE j IS ALIVE" << endl;}
    int me = uthread_get_tid();
    uthread_terminate(me);
}

//#include "uthreads.h"
//#include <cstdio>
//#include <stdio.h>
//#include <setjmp.h>
//#include <signal.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <iostream>
//#include <assert.h>


//
//#define SECOND 1000000
//
//void f(void) {
////    sigset_t maskedSet;
////    sigemptyset(&maskedSet);
////    sigaddset(&maskedSet, SIGVTALRM);
////    std::cerr << uthread_get_tid() << std::endl << std::flush;
////    sigprocmask(SIG_BLOCK, &maskedSet, NULL);
//    std::cerr << "started f" << std::endl;
//
//
////    sigprocmask(SIG_UNBLOCK, &maskedSet, NULL);
////    int j =  uthread_get_tid();
//    std::cerr << uthread_get_tid() << std::endl;
//    int i = 0;
//    while (true) {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
//
//        }
////        usleep(SECOND);
//    }
//}
//
//void f_terminating_itself(){
//    std::cerr << "started f" << std::endl;
//    int i = 0;
//    while (uthread_get_quantums(1) == 1) {
//        ++i;
//        if (i % 1000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
//        }
//    }
////    in the second run f terminates itself and should stop running
//    uthread_terminate(1);
//    while(true)
//    {
//        ++i;
//        if (i % 1000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(1) << std::endl;
//        }
//    }
//};
//
//void g(void) {
//    int i = 0;
//    std::cerr << "started g" << std::endl;
//
//    while (true) {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in g" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(2) << std::endl;
//        }
//    }
//}
//
//void f_with_sleep(void) {
//    std::cerr << "started f" << std::endl;
//    while (true) {
//        sleep(1);
//    }
//}
//
//void g_with_sleep(void) {
//    std::cerr << "started g" << std::endl;
//
//    while (true) {
//        sleep(1);
//    }
//}
//
//void f_for_block(void) {
//    std::cerr << "started f" << std::endl;
//    int i = 0;
////    blocking g_for_block
//    uthread_block(2);
////    it's an error to block the main thread
//    assert(uthread_block(0) == -1);
////    blocking tid which is blocked already is fine
//    assert(uthread_block(2) == 0);
//    while (true) {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//        }
////        usleep(SECOND);
//    }
//}
//
//void f_for_block2(void) {
//    std::cerr << "started f" << std::endl;
//    int i = 0;
////    blocking g_for_block
//    while(uthread_get_quantums(1) == 1)
//    {
//        ++i;
//        if (i % 10000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//        }
//    }
//    //        f blocks itself
//    uthread_block(1);
////        this resume shouldn't work
//    uthread_resume(1);
//    std::cerr << "f is back to life" << std::endl;
//    while(true)
//    {
//        ++i;
//        if (i % 10000000 == 0) {
//            std::cerr << "now i is " << i << " in f" << std::endl;
//        }
//    }
//}
//
//void g_for_block(void) {
//    int i = 0;
//    std::cerr << "started g" << std::endl;
//
//    while (true) {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in g" << std::endl;
//        }
////        usleep(SECOND);
//    }
//}
//
//void check_init() {
//    int array[3] = {-1, 2, 3};
//    assert(uthread_init(array, 3) == -1);
//}
//
//void check_spawn_bad_priority() {
//    int array[3] = {1, 2, 3};
//    uthread_init(array, 3);
//    assert(uthread_spawn(f, 3) == -1);
//}
//
//void check_spawn_limit() {
//    int array[3] = {1, 2, 3};
//    uthread_init(array, 3);
//    int i = 0;
//    while (i <= 98) {
//        std::cerr << uthread_spawn(f, 2) << std::endl;
//        ++i;
//    }
//    assert(uthread_spawn(f, 2) == -1);
//}
//
//void check_spawn_good() {
//    std::cerr << "started main thread" << std::endl<<std::flush;
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    assert(uthread_spawn(f, 1) == 1);
//    assert(uthread_spawn(g, 2) == 2);
////    std::cerr << "started main thread" << std::endl;
////    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
////    uthread_init(array, 3);
////    uthread_spawn(f, 1);
////    uthread_spawn(g, 2);
////    int i = 0;
////    while(true)
////    {
////        ++i;
////        if (i % 100000000 == 0) {
////            std::cerr << "now i is " << i << " in main thread" << std::endl;
////            std::cerr << "my quantum is: " << uthread_get_quantums(0) << std::endl;
////        }
////    }
//
//}
//
//void check_tid_recycle() {
//
//}
//
//void check_priority_change_bad() {
//    int array[3] = {1, 2, 3};
//    uthread_init(array, 3);
//    uthread_spawn(f, 1);
//    assert(uthread_change_priority(2, 2) == -1);
//    assert(uthread_change_priority(1, 3) == -1);
//}
//
//void check_priority_change_good() {
////    times should change from 400000 to 200000. need to check with prints
//    std::cerr << "started main thread" << std::endl<<std::flush;
//    int array[3] = {500000, 400000, 200000};
//    uthread_init(array, 3);
//    uthread_spawn(f, 1);
//    int i = 0;
//    while (i < 1000000000) {
//        ++i;
//    }
//    uthread_change_priority(1, 2);
//}
//
//void check_terminate1() {
////    should exit with exit code 0
//    int array[3] = {500000, 400000, 200000};
//    uthread_init(array, 3);
//    uthread_spawn(f, 1);
//    uthread_terminate(1);
//}
//
//// g is terminated imediately, then flow should go from f to main all back and forth
//void check_terminate2() {
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f, 1);
//    uthread_spawn(g, 2);
//    uthread_terminate(2);
////    tid2 shouldn't exist now
//    assert(uthread_change_priority(2, 2) == -1);
//    assert(uthread_get_tid() == 0);
//    while (true) {
//    }
//}
//
////checking a thread terminating itself (and it isn't the main thread)
////the run is: main, f, g, main, f (and then terminates itself), g, main, g, main...
//void check_terminate3() {
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f_terminating_itself, 1);
//    uthread_spawn(g, 2);
//    int i = 0;
//    while (true) {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in main thread" << std::endl;
//        }
//    }
//}
//
//// should print main thread output, then f thread, then g, and so on (indices should increase)
//void print_work_flow()
//{
//    std::cerr << "started main thread" << std::endl;
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f, 1);
//    uthread_spawn(g, 2);
//    int i = 0;
//    while(true)
//    {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in main thread" << std::endl;
//            std::cerr << "my quantum is: " << uthread_get_quantums(0) << std::endl;
//        }
//    }
//}
//
//void print_work_flow_with_sleep(){
//    std::cerr << "started main thread" << std::endl;
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f_with_sleep, 1);
//    uthread_spawn(g_with_sleep, 2);
//    int i = 0;
//    while(true)
//    {
//        ++i;
////        sleep(1);
////        std::cerr << "i in main is: " << i << std::endl;
////        if (i % 100000000 == 0) {
////            std::cerr << "now i is " << i << " in main thread" << std::endl;
////            std::cerr << "my quantum is: " << uthread_get_quantums(0) << std::endl;
////        }
//    }
//}
//
//// f block g when it starts running. hence, only f and main should run. then, when main runs again it then resumes g and
//// blocks f.
////so in total, the run is: main, f, main, g, main, g, ...
//void check_block1()
//{
//
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f_for_block, 1);
//    uthread_spawn(g_for_block, 2);
//    //    it's impossible to block a thread which doesn't exist
//    assert(uthread_resume(3) == -1);
////    it's impossible to block a thread which doesn't exist
//    assert(uthread_block(3) == -1);
//    int i = 0;
////    main started running and should have 1
//    assert( uthread_get_quantums(0) == 1);
//    while (uthread_get_quantums(0) == 1) {
//        ++i;
////        std::cerr << "i is: " << i << std::endl;
//        if (i % 10000000 == 0) {
//            std::cerr << "now i is " << i << " in main" << std::endl;
//        }
//    }
//    //        resuming g and blocking f
//    uthread_resume(2);
////    it's ok to resume thread which is already resumed
//    assert(uthread_resume(2) == 0);
////    it's ok to resume the running thread and has no effect
//    assert(uthread_resume(0) == 0);
//    uthread_block(1);
//    while(true)
//    {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in main" << std::endl;
//        }
//    }
//}
//
//// the run is: main, f, g, main, f (and then it blocks itself), g, main (here it resumes f), g, f, main, g, f...
//void check_block2()
//{
//    int array[3] = {5 * SECOND, 4 * SECOND, 3 * SECOND};
//    uthread_init(array, 3);
//    uthread_spawn(f_for_block2, 1);
//    uthread_spawn(g_for_block, 2);
//    int i = 0;
//    while(uthread_get_quantums(0) < 3)
//    {
//        ++i;
//        if (i % 1000000 == 0) {
//            std::cerr << "now i is " << i << " in main" << std::endl;
//        }
//    }
//    std::cerr << "main is now bringing f back to life" << std::endl;
//    uthread_resume(1);
//    while(true)
//    {
//        ++i;
//        if (i % 100000000 == 0) {
//            std::cerr << "now i is " << i << " in main" << std::endl;
//        }
//    }
//}
//
//int main() {
////    check_init();
////    check_spawn_bad_priority();
////    check_spawn_good(); //todo
////    check_spawn_limit();
////    check_priority_change_bad();
//    check_priority_change_good(); // todo
////    check_terminate1();
////    check_terminate2();
////    check_terminate3();
////check_tid_recycle();
////    print_work_flow();
////    check_block1();
////    check_block2();
////    print_work_flow_with_sleep(); //TODO waiting for checking sleep
//
//
//
//    while (true) {
//    }
//    return 0;
//}
//
//// **********************************************************************************
//
////
////#include "uthreads.h"
////#include <string>
////#include <iostream>
////#define FAILURE -1
////#define SUCCESS 0
////
////
////void printError(int result, int expected, const std::string& msg)
////{
////    if(result == expected)
////        return;
////    std::cerr << "==================================================" << std::endl;
////    std::cerr << "FAILED TEST WITH MSG " << msg << std::endl;
////    std::cerr << "==================================================" << std::endl;
////    exit(1);
////}
////
////void f(void)
////{
////    while(true) {}
////}
////
//////some simple tests
////int main()
////{
////    int result, id;
////    int wrongUsecs[] = {1000, -1, 200};
////    int wrongUsecs2[] = {1000, 0, 200};
////    int usecs[] = {1000, 500, 200};
////    //init test
////    result = uthread_init(usecs, -1);
////    printError(result, FAILURE, "you accepted negative size in function init when it shouldn't be allowed");
////    result = uthread_init(usecs, 0);
////    printError(result, FAILURE, "you accepted 0 size in function init when it shouldn't be allowed");
////    result = uthread_init(wrongUsecs, 3);
////    printError(result, FAILURE, "you accepted array with negative values in function init when it shouldn't be allowed");
////    result = uthread_init(wrongUsecs2, 3);
////    printError(result, FAILURE, "you accepted array with zero values in function init when it shouldn't be allowed");
////    result = uthread_init(usecs, 3);
////    printError(result, SUCCESS, "you failed when inputs were valid in function init");
////
////    //check usecs
////    result = uthread_get_total_quantums();
////    printError(result, 1, "you have a wrong starting quantums");
////    result = uthread_get_quantums(0);
////    printError(result, 1, "you have a wrong starting quantums for thread 0");
////    result = uthread_get_quantums(-1);
////    printError(result, FAILURE, "you returned quantums for thread with id -1");
////    result = uthread_get_quantums(1);
////    printError(result, FAILURE, "you returned quantums for thread with id 1");
////    result = uthread_get_quantums(MAX_THREAD_NUM);
////    printError(result, FAILURE, "you returned quantums for thread with id 1");
////
////    //check running
////    result = uthread_get_tid();
////    printError(result, 0, "the wrong thread is currently running");
////
////    //check spawn
////    result = uthread_spawn(f, -1);
////    printError(result, FAILURE, "you accepted negative priority in function spawn when it shouldn't be allowed");
////    result = uthread_spawn(f, 3);
////    printError(result, FAILURE, "you accepted priority bigger than size of usecs in function spawn when it shouldn't be allowed");
////    //check id's in spawn
////    for(int i = 1; i < MAX_THREAD_NUM; i++)
////    {
////        id = uthread_spawn(f, 0);
////        printError(id, i, "you failed when inputs were valid in function spawn");
////        result = uthread_block(id);
////        printError(result, SUCCESS, "you failed to block an existing thread in function block");
////    }
////    //can't create more id's because all of them are taken
////    id = uthread_spawn(f, 0);
////    printError(id, FAILURE, "you allowed to create more than MAX_THREAD_NUM id's in function spawn");
////
////    //delete a thread and check you get its id
////    result = uthread_terminate(51);
////    printError(result, SUCCESS, "you failed to terminate a non running thread");
////    id = uthread_spawn(f, 1);
////    printError(id, 51, "you failed to create the correct id in function spawn");
////    result = uthread_block(id);
////    printError(result, SUCCESS, "you failed to block an existing thread in function block");
////
////    //check priority
////    result = uthread_change_priority(51, -1);
////    printError(result, FAILURE, "you accepted a negative priority in function change priority");
////    result = uthread_change_priority(51, 3);
////    printError(result, FAILURE, "you accepted a priority bigger than size of usecs in function change priority");
////    result = uthread_change_priority(51, 0);
////    printError(result, SUCCESS, "you failed to change legal priority in function change priority");
////    result = uthread_change_priority(MAX_THREAD_NUM, 0);
////    printError(result, FAILURE, "you accepted an invalid id in function change priority");
////    result = uthread_change_priority(-1, 0);
////    printError(result, FAILURE, "you accepted an invalid id in function change priority");
////    result = uthread_terminate(51);
////    printError(result, SUCCESS, "you failed to terminate a non running thread");
////    result = uthread_change_priority(51, 0);
////    printError(result, FAILURE, "you accepted an invalid id in function change priority");
////
////    //testing block
////    result = uthread_block(-1);
////    printError(result, FAILURE, "you blocked thread -1");
////    result = uthread_block(MAX_THREAD_NUM);
////    printError(result, FAILURE, "you blocked thread with not existing id");
////    result = uthread_block(51);
////    printError(result, FAILURE, "you blocked thread with not existing id");
////    result = uthread_block(0);
////    printError(result, FAILURE, "you blocked thread 0");
////    result = uthread_block(1);
////    printError(result, SUCCESS, "you failed to block an existing thread");
////
////    //testing terminate
////    result = uthread_terminate(-1);
////    printError(result, FAILURE, "you accepted to terminate a non existing id");
////    result = uthread_terminate(MAX_THREAD_NUM);
////    printError(result, FAILURE, "you accepted to terminate a non existing id");
////    for(int i = 1; i < MAX_THREAD_NUM; i++)
////    {
////        if(i != 51)
////        {
////            result = uthread_terminate(i);
////            printError(result, SUCCESS, "you failed to terminate a non running thread");
////        }
////        else
////        {
////            result = uthread_terminate(i);
////            printError(result, FAILURE, "you accepted to terminate a non existing id");
////        }
////    }
////    std::cout << "successfully passed all tests" << std::endl;
////    uthread_terminate(0);
////}
