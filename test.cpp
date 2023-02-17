#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <vector>
#include <stdint.h>
#include <malloc.h>

using namespace std;

int main(){
	uint8_t* tmp = (uint8_t*)malloc(sizeof(uint8_t)*13);
	cout << malloc_usable_size(tmp) << endl;
	cout << sizeof(tmp) << endl;
}