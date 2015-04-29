#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <stack>
#include <math.h>
#include <vector>
#include <functional>
#include <queue>
#include <time.h>
#include <numeric>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>

#define datafile "d.txt"

using namespace std;

clock_t start, end;
vector<double> search_times;
vector<double> range_search_times;
vector<double> knn_search_times;

int bits, dim, pageSize, memory;

struct element{
	string obj; //0 for rect, 1 for pt
	float dist;
};

struct MyComparator{
    bool operator() (element *n1, element *n2){
         return (n1->dist) < (n2->dist);
    }
};

typedef priority_queue<element*, vector<element*>, MyComparator> myqueue;

float distance_(vector<float> p1, vector<float> p2){
	float res = 0;
	for(int i=0;i<p1.size();i++){
		float d = fabs(p1[i]-p2[i]);
		res += d*d;
	}
	return sqrt(res);
}

double sta_dev(vector<double> v, double mean){
	vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	return sqrt(sq_sum / v.size());
}

int search(vector<float> pt){
	string line;
	ifstream file1(datafile);
	if(file1.is_open()){
		while(getline(file1, line)){
			istringstream data(line);
			float temp; string object;
			int i = 0;
			for(i=0;i<pt.size();i++){
				data >> temp;
				if(pt[i]==temp);
				else break;
			}
			data >> object;
			if(i==pt.size()) return 1;
		}
	} else cout << "Error opening data file." << endl;
	return 0;
}

void range_search(vector<float> pt, float range, list<string>& res){
	string line;
	ifstream file1(datafile);
	if(file1.is_open()){
		while(getline(file1, line)){
			istringstream data(line);
			vector<float> tv;
			float temp; string object;
			int i = 0;
			for(i=0;i<pt.size();i++){
				data >> temp;
				tv.push_back(temp);
			}
			data >> object;
			if(distance_(pt,tv)<=range) res.push_back(object);
		}
	} else cout << "Error opening data file." << endl;
}

void knn_search(vector<float> pt, int k, myqueue *queue){
	if(k==0) return;
	string line;
	ifstream file1(datafile);
	if(file1.is_open()){
		while(getline(file1, line)){
			istringstream data(line);
			vector<float> tv;
			float temp; string object;
			int i = 0;
			for(i=0;i<pt.size();i++){
				data >> temp;
				tv.push_back(temp);
			}
			data >> object;
			element *e = new element;
			e->obj = object;
			e->dist = distance_(pt,tv);
			if(queue->size()<k){
				queue->push(e);
			} else {
				if(queue->top()->dist > e->dist){
					queue->pop();
					queue->push(e);
				}
			}
		}
	} else cout << "Error opening data file." << endl;
}

void query(string query_line, int flag){
	//if(flag>0)
	string args[2*dim+1];
	string arg;
	istringstream line(query_line);
	cout << query_line  << endl;
	int i = 0;
	while(line >> arg){
		args[i] = arg;
		i++;
	}
	
	int query_type = atoi(args[0].c_str());
	if(query_type==1){
		vector<float> pt;
		for(i=1;i<=dim;i++){
			pt.push_back((float) (atof(args[i].c_str())));
		}
		start = clock();
		int res = search(pt);
		end = clock();
		search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		if(res==0){
			cout << "Not found: (";					
			for(i=0;i<dim;i++) cout << args[i+1] << ",";
			cout << ") " << endl;
		}
		else{
			cout << "Found (";
			for(i=0;i<dim;i++) cout << args[i+1] << ",";
			cout << ") " << endl;
		}
	} else if(query_type==2){
		vector<float> pt;
		for(i=1;i<=dim;i++){
			pt.push_back(atof(args[i].c_str()));
		}
		float range = atof(args[i].c_str());
		list<string> res;
		start = clock();
		range_search(pt, range, res);
		end = clock();
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		for(list<string>::iterator it = res.begin(); it!=res.end();++it){
			cout << (*it) << endl;
		}
	} else if(query_type==3){
		vector<float> pt;
		for(i=1;i<=dim;i++){
			pt.push_back(atof(args[i].c_str()));
		}
		int k = (int) (atof(args[i].c_str()));
		myqueue queue;
		list<element*> res;
		start = clock();
		knn_search(pt, k, &queue);
		end = clock();
		knn_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		while(!queue.empty()){
			element *e = queue.top();
			res.push_front(e);
			queue.pop();
		}
		for(list<element*>::iterator it = res.begin(); it!=res.end();++it){
			element *e = (*it);
			cout << e->obj << endl;
		}
	}
	// if(flag>0)
	cout << "\n" ;
}

int main(){

	string query_line;
	ifstream config_file;
	config_file.open("vafile.config");
	config_file >> bits >> dim >> pageSize >> memory;
	//ifstream file3("aassgn4_r_querysample.txt");
	ifstream file3("q.txt");
	if(file3.is_open()){
		while(getline(file3, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening query file.." <<endl;

	ofstream output;
	double sum, mean, sq_sum, st_dev;
	output.open("linear.out");
	output << "All stats in milliseconds.\n";
	output << "\nPoint Search stats:\n";
	output << "min - " << *min_element(search_times.begin(), search_times.end()) << "\n";
	output << "max - " << *max_element(search_times.begin(), search_times.end()) << "\n";
	sum = std::accumulate(search_times.begin(), search_times.end(), 0.0);
	mean = sum / search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(search_times, mean) << "\n";
	output << "\nRange Search stats:\n";
	output << "min - " << *min_element(range_search_times.begin(), range_search_times.end()) << "\n";
	output << "max - " << *max_element(range_search_times.begin(), range_search_times.end()) << "\n";
	sum = std::accumulate(range_search_times.begin(), range_search_times.end(), 0.0);
	mean = sum / range_search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(range_search_times, mean) << "\n";
	output << "\nknn Search stats:\n";
	output << "min - " << *min_element(knn_search_times.begin(), knn_search_times.end()) << "\n";
	output << "max - " << *max_element(knn_search_times.begin(), knn_search_times.end()) << "\n";
	sum = std::accumulate(knn_search_times.begin(), knn_search_times.end(), 0.0);
	mean = sum / knn_search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(knn_search_times, mean) << "\n";

}