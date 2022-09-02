//
//  main.cpp
//  dlmq
//
//  Created by Fausto Saporito on 31/08/2022.
//

#include <iostream>
#include <pthread.h>
#include <queue>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cmath>

#define MAX 1
#define MAXREG 254
#define REGSIZE 36
#define PULSE 10000

#define WRM(x,y) inpv=(y);addr=(x);writef=1;this_thread::sleep_for(1.96s);writef=0;

#define RDM(x) this_thread::sleep_for(1s);addr=(x);readf=1;this_thread::sleep_for(2s);
 
using namespace std;
 
// Declaring global variables
int endp = 0;
int writef = 0;
int readf = 0;
int erasef = 0;
int outv = 0;
int inpv = 0;
int bk = 0;
int addr = 0;

int reg[MAXREG][REGSIZE];
 
// Shared queue
queue<int> Q;
queue<int> Q1;
queue<int> wQ;
queue<int> rQ;
 
// Function declaration of all required functions
void* producerFun(void*);
void* add_B(void*);
void* add_C(void*);
void* wrmem(void*);
 
// Getting the mutex
pthread_mutex_t mutexx = PTHREAD_MUTEX_INITIALIZER;
 
pthread_cond_t dataNotProduced =
                    PTHREAD_COND_INITIALIZER;
pthread_cond_t dataNotConsumed =
                    PTHREAD_COND_INITIALIZER;
 
// Function to generate random numbers and
// push them into queue using thread A
 
void* producerFun(void*)
{
    int num = 0;
     
    while (1) {
        // Getting the lock on queue using mutex
        pthread_mutex_lock(&mutexx);
 
        if (Q.size() < MAX)
        {
            num = !num;
            Q.push(num);
 
            pthread_cond_broadcast(&dataNotProduced);
        }
 
        // If queue is full, release the lock and return
        else if (endp) {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        } else {
            cout << ".";
            pthread_cond_wait(&dataNotConsumed, &mutexx);
        }
        
        // Get the mutex unlocked
        pthread_mutex_unlock(&mutexx);
    }
}
 
// Function definition for consumer thread B
// clock divider
void* add_B(void*)
{
    int count = 0;
//    int cdiv = 0;
    while (1) {
 
        // Getting the lock on queue using mutex
        pthread_mutex_lock(&mutexx);
 
        // Pop only when queue has at least 1 element
        if (Q.size() > 0) {
            // Get the data from the front of queue
            int data = Q.front();
 
            //cout << "B thread consumed: " << data << endl;
            if (data == 1) {
//                if (cdiv == PULSE) {
//                    count = (count < REGSIZE) ? (count + 1) : 0;
                this_thread::sleep_for(50ms);
                count = (count + 1) % REGSIZE;
//                cout << count << endl;
                Q1.push(count);
//                    cdiv = 0;
//                }
//                cdiv++;
            }
 
            // Pop the consumed data from queue
            Q.pop();
 
            pthread_cond_signal(&dataNotConsumed);
        }
        else if (endp) {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        } else {
            cout << "C";
            pthread_cond_wait(&dataNotProduced, &mutexx);
        }
        
        // Get the mutex unlocked
        pthread_mutex_unlock(&mutexx);
    }
}
 
// Function definition for consumer thread C
void* add_C(void*)
{
    int data = 0;
    while (1) {
        // Getting the lock on queue using mutex
        pthread_mutex_lock(&mutexx);
 
        // Pop only when queue has at least 1 element
        if (Q1.size() > 0) {
 
            // Get the data from the front of queue
            data = Q1.front();
//            cout << "Analyzing queue: " << data << inpv << writef << endl;
            if (data == addr) {
                cout << "A" << addr;
                reg[bk][data] = (inpv & writef) ^ (reg[bk][data] & (!erasef));
//                cout << "Reg " << reg[bk][data] << endl;
                outv = reg[bk][data] & readf;
//                cout << "Reading: " << outv << endl;
            }
//                cout << "R" << readf << addr << outv << endl;
//                printf("%d|\n",reg[0][addr]);
 
            // Pop the consumed data from queue
            Q1.pop();
 
            pthread_cond_signal(&dataNotConsumed);
        }
        else if (endp)
        {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        } else {
            cout << "R";
            // Wait on a condition
            pthread_cond_wait(&dataNotProduced, &mutexx);
        }
 
        // Get the mutex unlocked
        pthread_mutex_unlock(&mutexx);
    }
}

int rdmem(int b, int a)
{
    a = a + 2;
    int r = 0;
    while (1) {
        if (addr == (a)) {
            readf = 1;
            this_thread::sleep_for(35ms);
            r = outv;
            this_thread::sleep_for(35ms);
            readf = 0;
            return r;
        }
    }
}

void *wrmem(void*)
{
    int ad;
    while (1) {
        pthread_mutex_lock(&mutexx);
 
        // Pop only when queue has at least 1 element
        if (wQ.size() > 0) {
            ad = wQ.front();
            cout << "wrm " << ad << writef << endl;
            reg[bk][ad] = (inpv & writef) ^ (reg[bk][ad] & (!erasef));
            wQ.pop();
        } else if (endp) {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        }
        
        pthread_mutex_unlock(&mutexx);
    }
    
}
 
// Driver code
int main()
{
    // Declaring integers used to
    // identify the thread in the system
    pthread_t producerThread, consumerThread1, consumerThread2;
 
    // Function to create a threads
    // (pthread_create() takes 4 arguments)
    pthread_create(&producerThread, NULL, producerFun, NULL);
    pthread_create(&consumerThread1,NULL, *add_B, NULL);
    pthread_create(&consumerThread2,NULL, *add_C, NULL);

    /*
    inpv = 1;
    addr = 2;
    writef = 1;
    this_thread::sleep_for(2s);
    writef = 0;
    
    inpv = 1;
    addr = 3;
    writef = 1;
    this_thread::sleep_for(2s);
    writef = 0;
    */
    
//    WRM(0,1)
    WRM(1,1)
    WRM(2,1)
    WRM(3,1)
    WRM(4,1)
    WRM(5,1)
    WRM(6,1)
    WRM(7,1)
    WRM(8,1)
    WRM(9,1)
    WRM(10,1)
    WRM(11,1)
    WRM(12,1)
    WRM(13,1)
    WRM(14,1)
    WRM(15,1)
    WRM(16,1)
    WRM(17,1)
    WRM(18,1)
    WRM(19,1)
    WRM(20,1)
    WRM(21,1)
    WRM(22,1)
    WRM(23,1)
    WRM(24,1)
    WRM(25,1)
    WRM(26,1)
    WRM(27,1)
    WRM(28,1)
    WRM(29,1)
    WRM(30,1)
    WRM(31,1)
    WRM(32,1)
    WRM(33,1)
    WRM(34,1)
    WRM(35,1)
    
//    Reading
    
    /*
    this_thread::sleep_for(1s);
    addr = 2;
    readf = 1;
    this_thread::sleep_for(2s);
     */
    
    RDM(1)
    printf("\nOutput is %d\n", outv);
    RDM(2)
    printf("\nOutput is %d\n", outv);
    RDM(3)
    printf("\nOutput is %d\n", outv);
    RDM(4)
    printf("\nOutput is %d\n", outv);
    RDM(5)
    printf("\nOutput is %d\n", outv);
    RDM(6)
    printf("\nOutput is %d\n", outv);
    RDM(7)
    printf("\nOutput is %d\n", outv);
    RDM(8)
    printf("\nOutput is %d\n", outv);
    RDM(9)
    printf("\nOutput is %d\n", outv);
    RDM(10)
    printf("\nOutput is %d\n", outv);
    RDM(11)
    printf("\nOutput is %d\n", outv);
    RDM(12)
    printf("\nOutput is %d\n", outv);
    RDM(13)
    printf("\nOutput is %d\n", outv);
    RDM(14)
    printf("\nOutput is %d\n", outv);
    RDM(15)
    printf("\nOutput is %d\n", outv);
    RDM(16)
    printf("\nOutput is %d\n", outv);
    RDM(17)
    printf("\nOutput is %d\n", outv);
    RDM(18)
    printf("\nOutput is %d\n", outv);
    RDM(19)
    printf("\nOutput is %d\n", outv);
    RDM(20)
    printf("\nOutput is %d\n", outv);
    RDM(21)
    printf("\nOutput is %d\n", outv);
    RDM(22)
    printf("\nOutput is %d\n", outv);
    RDM(23)
    printf("\nOutput is %d\n", outv);
    RDM(24)
    printf("\nOutput is %d\n", outv);
    RDM(25)
    printf("\nOutput is %d\n", outv);
    RDM(26)
    printf("\nOutput is %d\n", outv);
    RDM(27)
    printf("\nOutput is %d\n", outv);
    RDM(28)
    printf("\nOutput is %d\n", outv);
    RDM(29)
    printf("\nOutput is %d\n", outv);
    RDM(30)
    printf("\nOutput is %d\n", outv);
    RDM(31)
    printf("\nOutput is %d\n", outv);
    RDM(32)
    printf("\nOutput is %d\n", outv);
    RDM(33)
    printf("\nOutput is %d\n", outv);
    RDM(34)
    printf("\nOutput is %d\n", outv);
    RDM(35)
    printf("\nOutput is %d\n", outv);

//    this_thread::sleep_for(10s);
    
    endp = 1;
    printf("\n0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5\n");
    
    for (int i=0;i<REGSIZE;i++) printf("%d ",reg[0][i]);
//    printf("%d\n", s);
    
    return 0;
}
