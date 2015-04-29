#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdlib.h>
#include <list>
#include <math.h>
#include <vector>
#include <functional>
#include <queue>
#include <time.h>
#include <numeric>
#include <iomanip>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]){
	int arg = atoi(argv[1]);
	char * file = (char *) malloc(15*sizeof(char));
	sprintf(file, "node%d.bin", arg);
	int key = 0;
	cout << file << endl;
	/*ofstream file1;
	file1.open(file.c_str(), ios::out | ios::binary);
	key++;
	file1.write((char*)&key,sizeof(key));
	key++;
	file1.write((char*)&key,sizeof(key));key++;
	    file1.write((char*)&key,sizeof(key));key++;
	    file1.write((char*)&key,sizeof(key));key++;
	    file1.write((char*)&key,sizeof(key));key++;
	    file1.write((char*)&key,sizeof(key));
	file1.close();
	int p = 0;*/
	ifstream file2(file,ios::binary);
	//file2.open(file.c_str(),ios::binary);
	if(file2.is_open()){
		//file2.seekg(p, ios::beg);
		while(file2.read((char*)&key,sizeof(key))){
			cout << key << endl;
		}
		/*	string a;
		while(file2 >> a){
			cout << a << endl;
		}*/
	}
	cout << "--------------------" << endl;
	file2.close();/*
	fstream file3(file.c_str(), ios::in | ios::out | ios::binary);
	file3.seekp(p-4, ios::beg);
	key = 31;
	file3.write((char*)&key,sizeof(key));
	file3.close();
	file2.open(file.c_str(),ios::binary);
		//file2.seekg(p, ios::beg);
		while(file2.read((char*)&key,sizeof(key))){
			cout << key << endl;
		}
	*/

}
