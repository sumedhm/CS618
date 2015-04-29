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

using namespace std;

clock_t start, end;
vector<double> search_times;
vector<double> range_search_times;
vector<double> knn_search_times;

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

int bits, dim, pageSize;
long memory, fileSize;
bool allRead, finish; int chunk;
int index_size, partitions, padding;

int objects = 1;
vector<string> to_binary;
string vafilename =  "vafile.bin";

vector<string> indices;
vector<int> obj_files;

long GetFileSize(string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

float distance_(vector<float> p1, vector<float> p2){
	float res = 0;
	for(int i=0;i<p1.size();i++){
		float d = fabs(p1[i]-p2[i]);
		res += d*d;
	}
	return sqrt(res);
}

int str_to_int(string str){
	int ans = 0;
	int i = str.length()-1,j=0;
	while(i>=0){
		if(str[i]=='0');
		else ans += pow(2, j);
		i--;j++;
	}
	return ans;
}

float rect_distance(string point,vector<float> pt){
	int i=0, j=0;
	float res = 0;
	while(i<point.length()){
		stringstream ss;
		ss << point[i] << point[i+1];
		string temp = ss.str();
		int part = str_to_int(temp);
		float min = part*(1.0/partitions);
		float max = (part+1)*(1.0/partitions);
		float d = 0;
		if(pt[j]<min) d = min - pt[j];
		else if(pt[j]>max) d = pt[j] - max;
		else d = 0;
		res += d*d;
		i += 2; j++;
	}
	return sqrt(res);
}

int check_intersection(string point,vector<float> pt,float range){
	float res = rect_distance(point, pt);
	if(res>range) return 0;
	return 1;
}

string point_to_string(vector<float> point){

	string ans = "";
	for(int i=0;i<dim;i++){
		int temp = floor(point[i]*partitions);
		//cout << "point - " << point[i] << ", bin - " << temp << endl;
		ans += to_binary[temp];
	}
	while(ans.length()<index_size){
		ans += "0";
	}
	return ans;

}

char byte_to_char(string point, int start, int end){
	int i = end, j = 0, ans = 0;
	while(i>=start){
		if(point[i]=='0');
		else ans += pow(2,j);
		i--; j++;
	}
	//cout << ans;
	return (char)ans;
}

string int_to_byte(int obj, int size){
	string ans = "";
	while(obj>0){
		if(obj%2==0) ans = "0"+ans;
		else ans = "1"+ans;
		obj /= 2;
	}
	while(ans.length()<size){
		ans = "0"+ans;
	}

	return ans;
}

string byte_to_str(vector<int> code){
	string ans = "";
	for(int i=0;i<code.size();i++){
		ans += int_to_byte(code[i],8);
	}
	return ans;
}

void insert(vector<float> pt, string object){

	string point = point_to_string(pt);
	//cout << point << endl;
	ofstream vafile;
	vafile.open(vafilename.c_str(), ios::binary | ios::app | ios::out);
	for(int i=0;i<index_size/8;i++){
		char temp = byte_to_char(point, i*8, ((i+1)*8)-1);
		//cout << "-"<< temp << ",";
		vafile.write((char *) &temp, sizeof(temp));		
	}

	point = int_to_byte(objects,32);
	for(int i=0;i<4;i++){
		char temp = byte_to_char(point, i*8, ((i+1)*8)-1);
		//cout << "-"<< temp << ",";
		vafile.write((char *) &temp, sizeof(temp));
	}
	vafile.close();
	char *file;
	file = (char *) malloc (15*sizeof(char));
	sprintf(file, "obj%d.bin", objects);
	objects++;
	ofstream file2;
	file2.open(file, ios::out | ios::binary);
	for(int i=0;i<pt.size();i++){
		file2 << pt[i] << " ";
	}
	file2 << object;
	file2 << "\n";
	file2.close();
	//cout << point << endl;
}

void readfile(vector<string>& indices, vector<int>& obj_files){
	int obj;
	ifstream vafile(vafilename.c_str(), ios::in|ios::binary|ios::ate);
	vafile.seekg (0, vafile.end);
    int length = vafile.tellg();
    vafile.seekg (0, vafile.beg);
	char *buffer = new char[length];
	// cout << length << "l" << endl;
	vafile.read(buffer,length);
	int l=0;
	while(l<length){
		vector<int> code;
	    for(int i=0;i<index_size/8;i++){
	    	code.push_back((int) buffer[l]);
	    	l++;
	    }
	    indices.push_back(byte_to_str(code));
	    code.clear();
	    for(int i=0;i<4;i++){
	    	code.push_back((int) buffer[l]);
	    	l++;
	    }
	    string str = byte_to_str(code);
	    obj_files.push_back(str_to_int(str));
	    code.clear();
	    //cout << indices[0] << "----" << obj_files[0] << endl;
	}   
}

void readnext(vector<string>& indices, vector<int>& obj_files){
	indices.clear();
	obj_files.clear();
	int obj;
	ifstream vafile(vafilename.c_str(), ios::in|ios::binary|ios::ate);
	int length = memory;
	vafile.seekg (length*chunk, vafile.beg);
    char *buffer = new char[length];
	// cout << length << "l" << endl;
	vafile.read(buffer,length);
	if(length*(chunk+1)>=fileSize){
		finish = true;
	}
	int l=0;
	while(l<length){
		vector<int> code;
	    for(int i=0;i<index_size/8;i++){
	    	code.push_back((int) buffer[l]);
	    	l++;
	    }
	    indices.push_back(byte_to_str(code));
	    code.clear();
	    for(int i=0;i<4;i++){
	    	code.push_back((int) buffer[l]);
	    	l++;
	    }
	    string str = byte_to_str(code);
	    obj_files.push_back(str_to_int(str));
	    code.clear();
	    //cout << indices[0] << "----" << obj_files[0] << endl;
	}
    
}

int search(vector<float> pt){
	string point = 	point_to_string(pt);
	for(int i=0;i<indices.size();i++){
		if(indices[i]==point){
			char *file;
			file = (char *) malloc (15*sizeof(char));
			sprintf(file, "obj%d.bin", obj_files[i]);
			ifstream object_file;
			object_file.open(file);
			float temp;int j;
			for(j=0;j<pt.size();j++){
				object_file >> temp;
				if(pt[j]==temp);
				else break;
			}
			if(j==pt.size()) return 1;
		}
	}
	return 0;
}

void range_search(vector<float> pt, float range, list<string>& res){
	string point = 	point_to_string(pt);
	for(int i=0;i<indices.size();i++){
		if(indices[i]==point){
			vector<float> temp;
			char *file;
			file = (char *) malloc (15*sizeof(char));
			sprintf(file, "obj%d.bin", obj_files[i]);
			ifstream object_file;
			object_file.open(file);
			int j; string object; float t;
			for(j=0;j<pt.size();j++){
				object_file >> t;
				temp.push_back(t);
			}
			object_file >> object;
			float dist = distance_(pt, temp);
			if(dist <= range){
				res.push_back(object);
			}
		} else {
			int in = check_intersection(indices[i], pt, range);
			if(in==1){
				vector<float> temp;
				char *file;
				file = (char *) malloc (15*sizeof(char));
				sprintf(file, "obj%d.bin", obj_files[i]);
				ifstream object_file;
				object_file.open(file);
				int j; string object; float t;
				for(j=0;j<pt.size();j++){
					object_file >> t;
					temp.push_back(t);
				}
				object_file >> object;
				float dist = distance_(pt, temp);
				if(dist <= range){
					res.push_back(object);
				}
			}
		}
	}	
}

void knn_search(vector<float> pt, int k, myqueue *queue){
	for(int i=0;i<indices.size();i++){
		if(queue->size() < k){
			vector<float> temp;
			char *file;
			file = (char *) malloc (15*sizeof(char));
			sprintf(file, "obj%d.bin", obj_files[i]);
			ifstream object_file;
			object_file.open(file);
			int j; string object; float t;
			for(j=0;j<pt.size();j++){
				object_file >> t;
				temp.push_back(t);
			}
			object_file >> object;
			element *e = new element;
			e->obj = object;
			e->dist = distance_(temp,pt);
			queue->push(e);
		} else {
			if(rect_distance(indices[i],pt) > queue->top()->dist);
			else {
				vector<float> temp;
				char *file;
				file = (char *) malloc (15*sizeof(char));
				sprintf(file, "obj%d.bin", obj_files[i]);
				ifstream object_file;
				object_file.open(file);
				int j; string object; float t;
				for(j=0;j<pt.size();j++){
					object_file >> t;
					temp.push_back(t);
				}
				object_file >> object;
				element *e = new element;
				e->obj = object;
				e->dist = distance_(temp,pt);
				if(queue->top()->dist > e->dist){
					queue->pop();
					queue->push(e);
				}
			}
		}
	}
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
	
	if(query_type==0){
		vector<float> pt;
		for(i=1;i<=dim;i++){
			pt.push_back((atof(args[i].c_str())));
		}
		//start = clock();
		insert(pt,args[i]);
		//end = clock();
		//if(flag==1) insert_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		//if(flag>0)
		/*cout << "Inserted: (";
		for(
		i=0;i<d;i++) cout << args[i+1] << ",";
		cout << ") " << args[i+1] << endl;*/
	} else if(query_type==1){
		vector<float> pt;
		for(i=1;i<=dim;i++){
			pt.push_back((float) (atof(args[i].c_str())));
		}
		start = clock();
		int res = 0;
		finish = false;
		chunk = 0;
		if(allRead){
			res = search(pt);
		}
		else {
			while(!finish && res==0){
				readnext(indices, obj_files);
				res = search(pt);
				chunk++;
			}
		}
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
		if(allRead){
			range_search(pt, range, res);
		}
		else {
			finish = false;
			chunk = 0;		
			while(!finish){
				readnext(indices, obj_files);
				range_search(pt, range, res);
				chunk++;
			}
		}
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
		if(allRead){
			knn_search(pt, k, &queue);
		}
		else {
			finish = false;
			chunk = 0;		
			while(!finish){
				readnext(indices, obj_files);
				knn_search(pt, k, &queue);
				chunk++;
			}
		}
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

string int_to_string(vector<int> int_array){
  ostringstream oss("");
  for (int temp = 0; temp < int_array.size(); temp++)
    oss << int_array[temp];
  return oss.str();
}

double sta_dev(vector<double> v, double mean){
	vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	return sqrt(sq_sum / v.size());
}

int main(){

	string query_line;
	ifstream config_file;
	config_file.open("vafile.config");
	config_file >> bits >> dim >> pageSize >> memory;
	partitions = pow(2, bits);
	int temp = dim*bits;
	int i = 1;
	while(1){
		index_size = 8*i;
		if(index_size > temp){
			padding = index_size-temp;
			break;
		}
		i++;
	}
	memory = memory * 1024*1024*1024;
	// memory = 24;
	int bytes = index_size/8 + 4;
	if(memory%bytes!=0){
		memory = memory - (memory%bytes);
	}
	for(i=0;i<partitions;i++){
		vector<int> binary;
		temp = i;
		for(int j=0;j<bits;j++){
			binary.insert(binary.begin(), temp%2);
			temp /= 2;
		}
		to_binary.push_back(int_to_string(binary));
	}

	//ifstream file1("assgn6_data_exp.txt");
	ifstream file1("d.txt");
	if(file1.is_open()){
		while(getline(file1, query_line)){
			query("0	" + query_line, 0);
		}
	} else cout << "Error opening data file." << endl;
	fileSize = GetFileSize("vafile.bin");
	if(fileSize < memory){
		readfile(indices, obj_files);
		allRead = true;
	} else {
		allRead = false;
	}
	//ifstream file3("aassgn4_r_querysample.txt");
	ifstream file3("q.txt");
	if(file3.is_open()){
		while(getline(file3, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening query file.." <<endl;

	ofstream output;
	double sum, mean, sq_sum, st_dev;
	output.open("vafile.out");
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