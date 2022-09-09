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
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <fmt/core.h>

#define MAX 1
#define MAXREG 256
#define REGSIZE 36
#define PULSE 10000

#define WRM(b,x,y) bk=b;this_thread::sleep_for(0.2s);inpv=(y);addr=(x);writef=1;this_thread::sleep_for(1.96s);writef=0;
#define DEL(x) addr=(x);erasef=1;this_thread::sleep_for(1.96s);erasef=0;
#define RDM(b,x) bk=b;this_thread::sleep_for(1s);addr=(x);readf=1;this_thread::sleep_for(2s);
 
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
long ac = 0;

int a[36];
vector<int> mule;
int acb[36];

int reg[MAXREG][REGSIZE];

regex regf("\\s+");
 
// Shared queue
queue<int> Q;
queue<int> Q1;
 
// Function declaration of all required functions
void* producerFun(void*);
void* add_B(void*);
void* add_C(void*);
 
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
            //cout << "." << flush;
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
    
    while (1) {
 
        // Getting the lock on queue using mutex
        pthread_mutex_lock(&mutexx);
 
        // Pop only when queue has at least 1 element
        if (Q.size() > 0) {
            // Get the data from the front of queue
            int data = Q.front();
 
            if (data == 1) {
                this_thread::sleep_for(50ms);
                count = (count + 1) % REGSIZE;
                Q1.push(count);
            }
 
            // Pop the consumed data from queue
            Q.pop();
 
            pthread_cond_signal(&dataNotConsumed);
        }
        else if (endp) {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        } else {
            //cout << "C" << flush;
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
            if (data == addr) {
                cout << bk << "#A" << addr << "." << flush;
                reg[bk][data] = (inpv & writef) ^ (reg[bk][data] & (!erasef));
                outv = reg[bk][data] & readf;
            }
 
            // Pop the consumed data from queue
            Q1.pop();
 
            pthread_cond_signal(&dataNotConsumed);
        }
        else if (endp)
        {
            pthread_mutex_unlock(&mutexx);
            return NULL;
        } else {
            //cout << "R" << flush;
            // Wait on a condition
            pthread_cond_wait(&dataNotProduced, &mutexx);
        }
 
        // Get the mutex unlocked
        pthread_mutex_unlock(&mutexx);
    }
}

void dec2bin(long n)
{
	for(int i=0;i<36;i++) a[i] = 0;
	for(int i=0;n>0;i++) {    
		a[35-i] = n%2;    
		n = n/2;  
	}
}

long bin2dec() {
    long t = 0;
	for (int i=0;i<36;i++) {
		t += acb[i] * pow(2,35-i);
	}
	return t;
}

void sto(int b,long v)
{
	dec2bin(v);
	for (int i=1;i<36;i++) {
		WRM(b,i,a[i])
    }
}

long lda(int b)
{
	for(int i=0;i<36;i++) a[i] = 0;
	for (int i=1;i<36;i++) {
        RDM(b,i)
        a[i]=outv;
    }
    return bin2dec();
}

long add(int b1, int b2, int b3)
{
	int c = 0;
	int s = 0;
	int bb1 = 0;
	int bb2 = 0;
	
	for(int i=0;i<REGSIZE;i++) {
		RDM(b1,35-i)
		bb1 = outv;
		RDM(b2,35-i)
		bb2 = outv;
		s = c + bb1 + bb2;
		if(s>=2) {
			s=0; c=1;
		} else { 
			c=0;
		}
		WRM(b3,35-i,s)
		acb[35-i] = s;
	}
	
	if (c==1) {
		WRM(b3,1,1)
		acb[1] = 1;
	}
	
	ac = bin2dec();
    
    return ac;
}

int digits(int b1)
{
	for(int i=0;i<REGSIZE;i++) {
		RDM(b1,i)
		if (outv == 1) return i;
	}
	return 0;
}

void mul(int b1, int b2) 
{
	int m1 = 0;
	int m2 = 0;
	int m = 0;
	int maxj = 0;
	int treg = 0;
	int bb = 256;
	
	int cf1 = digits(b1);
	int cf2 = digits(b2);
	
	cout << "D" << cf1 << "." << cf2 << flush;
	
	for(int j=cf2;j<REGSIZE;j++) {
		RDM(b2,j)
		m2 = outv;
		bb--;
		for(int i=0;i<cf1;i++) {
			RDM(b1,35-i)
			m1 = outv;
			WRM(bb,35-i-treg,m1*m2)
		}
		treg++;
	}
	
	for (int i=0;i<REGSIZE-cf2;i=i+2) {
		//add(255-i,255-i-1,100+i-1);
		//mule[i] = 100+i;
		cout << "\n" << 255-i << "+" << 255-i-1 << "->" << 100+i-1 << endl;
	}
	
}

void lsh(int b1, int b2, int n)
{
	for(int i=0;i<REGSIZE;i++) {
		WRM(b2,i,0)
	}
	
	for(int i=0;i<REGSIZE;i++) {
		RDM(b1,i)
		WRM(b2,35-i-n,outv)
	}
}

void rsh(int b1, int b2, int n)
{
	for(int i=0;i<REGSIZE;i++) {
		WRM(b2,i,0)
	}
	
	for(int i=n+1;i<REGSIZE;i++) {
		RDM(b1,i)
		WRM(b2,35-i,outv)
	}
}

void cla()
{
	ac=0;
}

void preg(void)
{
    const auto fmtstr = "R[{:03d}]: ";
    const auto acfmt = "AC: {:09d}";
    
	printf("\n        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5\n");
	for (int j=0;j<MAXREG;j++) {
		cout << fmt::format(fmtstr,j);
    	for (int i=0;i<REGSIZE;i++) printf("%d ",reg[j][i]);
    	cout << endl;
	}
	cout << fmt::format(acfmt,ac) << endl;
}
 
// Driver code
int main()
{
    // Declaring integers used to
    // identify the thread in the system
    pthread_t producerThread, consumerThread1, consumerThread2;
    
    for(int i=0;i<MAXREG;i++) {
    	for(int j=0;j<REGSIZE;j++) {
    		reg[i][j] = 0;
    	}
    }
 
    // Function to create a threads
    // (pthread_create() takes 4 arguments)
    pthread_create(&producerThread, NULL, producerFun, NULL);
    pthread_create(&consumerThread1,NULL, *add_B, NULL);
    pthread_create(&consumerThread2,NULL, *add_C, NULL);
    
    string filename("input.txt");
    vector<string> lines;
    string line;

    ifstream input_file(filename);
    
    while (getline(input_file, line)){
        lines.push_back(line);
    }
    
    for (const auto &i : lines){
    	sregex_token_iterator iter(i.begin(), i.end(), regf, -1);
    	sregex_token_iterator end;

    	vector<string> vec(iter, end);
    	
    	for (int i=0;i<vec.size();i++)
    	{
        	if (vec[i].compare("STO") == 0) {
        		auto bb = stoi(vec[++i]);
        		auto va = stoi(vec[++i]);
        		cout << va << endl;
        		sto(bb,va);
        	} else if (vec[i].compare("ADD") == 0) {
        		auto bb1 = stoi(vec[++i]);
        		auto bb2 = stoi(vec[++i]);
        		auto bb3 = stoi(vec[++i]);
        		cout << bb1 << bb2 << bb3 << endl;
        		add(bb1,bb2,bb3);
        	} else if (vec[i].compare("///") == 0) {
        		endp = 1;
        	} else if (vec[i].compare("MUL") == 0) {
        		auto bb1 = stoi(vec[++i]);
        		auto bb2 = stoi(vec[++i]);
        		cout << bb1 << bb2 << endl;
        		mul(bb1,bb2);
        		for(auto j : mule) cout << j << "#" << endl;
        	}
    	}
    }
    
    endp = 1;
    preg();
    input_file.close();
    return 0;
}
