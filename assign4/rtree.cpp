#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
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

using namespace std;

int pageSize, d;

int root;
int cnt = 1;
int objects = 1;
int leafCapacity;
int nodeCapacity;
int padding, padding_i;

clock_t start, end;
vector<double> insert_times;
vector<double> search_times;
vector<double> range_search_times;
vector<double> knn_search_times;
vector<double> window_search_times;



struct element{
	int type; //0 for rect, 1 for pt
	double dist;
	int data;
};

struct MyComparator{
    bool operator() (element *n1, element *n2){
         return (n1->dist) > (n2->dist);
    }
};

typedef priority_queue<element*, vector<element*>, MyComparator> myqueue;

int new_node(int node_type){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", cnt++);
	ofstream file1;
	file1.open(file, ios::out | ios::binary);
	int data = 1;
	for(int i=1;i<=(pageSize/4);i++){
		if(i==1) data = node_type;
		else if(i<=padding) data = 0;
		else data = 1000001;
		file1.write((char*)&data,sizeof(data));
	}
	file1.close();
	return (cnt-1);

}

int new_buff(vector<int>& v, int type){

	int data;
	for(int i=1;i<=(pageSize/4);i++){
		if(i==1) data = type;
		else if(i<=padding) data = 0;
		else data = 1000001;
		v.push_back(data);
	}
	return 1;
}


int buff_to_file(char *file, vector<int>& buff){
	//cout << file << endl;
	ofstream file3(file, ios::out | ios::binary);
	for(int i=0;i<buff.size();i++){
		//cout << buff[i] << endl;
		file3.write((char*)&buff[i],sizeof(buff[i]));
	}

	file3.close();
}

int file_to_buff(char *file, vector<int>& buff){
	int data;
	ifstream file1(file, ios::in | ios::out | ios::binary);
    if(file1.is_open()){
	    while(file1.read((char*)&data,sizeof(data))){
	    	buff.push_back(data);
	    }
	} else return 0;
	file1.close();
	return 1;
}

void add_object(list<string>& queue, int objId){
	ifstream objFile;
	char *temp;
	temp = (char *) malloc (15*sizeof(char));
	sprintf(temp, "obj%d.bin", objId);
	objFile.open(temp);
	string obj;
	objFile >> obj;
	queue.push_back(obj);
}

void update_mbr(vector<int>& v){

	int dim = 0;
	int seek;
	int min, max;
	while(dim < d){
		seek = padding + dim;
		min = v[seek];
		max = min;
		seek = seek + d + 1;
		while(seek < v.size()){
			if(min > v[seek]) min = v[seek];
			if(v[seek]!=1000001 && max < v[seek]) max = v[seek];
			seek = seek + d + 1;
		}
		v[dim+dim+1] = min;
		v[dim+dim+2] = max;
		dim++;
	}

	return;
}

int volume_mbr(vector<int>& v){
	int dim = 0;
	long long int vol = v[2]-v[1];
	for(dim=1;dim<d;dim++){
		vol *= v[dim+dim+2] - v[dim+dim+1];
	}
	return vol;
}

int volume_min(vector<int>& buff, int ptr1, int ptr2, int seek, vector<int>& newbuff, vector<int>& anotherbuff){

	if(seek == ptr1 || seek == ptr2) return 0;
	int v1 = volume_mbr(newbuff);
	int i = padding;
	while(i<newbuff.size()){
		if(newbuff[i]>1000000){
			for(int j=0;j<=d;j++){
				newbuff[i+j] = buff[seek+j];
			}
			break;
		}
		i += d+1;
	}
	update_mbr(newbuff);
	int v2 = volume_mbr(newbuff);
	int diff1, diff2;
	diff1 = v2 - v1;
	v1 = volume_mbr(anotherbuff);
	i = padding;
	while(i<anotherbuff.size()){
		if(anotherbuff[i]>1000000){
			for(int j=0;j<=d;j++){
				anotherbuff[i+j] = buff[seek+j];
			}
			break;
		}
		i += d+1;
	}
	update_mbr(anotherbuff);
	v2 = volume_mbr(anotherbuff);
	diff2 = v2 - v1;
	if(diff1 > diff2) return 2;
	return 1;
}

int minimum(int a, int b){
	return ((a<b) ? a : b);
}

int maximum(int a, int b){
	return ((a>b) ? a : b);
}

double distance(vector<int> pt,vector<int> buff, int seek){
	int dis;
	double dist=0;
	for(int i=0;i<d;i++){
		dis = abs(buff[seek+i] - pt[i]);
		double x = (double)dis/1000000;
		dist = dist + (x)*(x);
	}
	return sqrt(dist);
}

double dist_rect(vector<int> pt,vector<int> buff, int seek){
	double dis;
	double dist=0;
	for(int i=0;i<d;i++){
		if(buff[seek+i+i]>pt[i]) dis = buff[seek+i+i] - pt[i];
		else if(buff[seek+i+i+1]<pt[i]) dis = pt[i] - buff[seek+i+i+1];
		else dis = 0;
		double x = (double)dis/1000000;
		dist = dist + (x)*(x);
	}
	return sqrt(dist);
}

int check_intersection(vector<int> pt, int range, vector<int> buff, int seek){
	double dist = dist_rect(pt,buff,seek);
	if(dist <= (double)range/1000000) return 1;
	return 0;
}

void update_mbr_internal(vector<int>& v){

	int dim = 0;
	int seek;
	int min, max;
	while(dim < d){
		seek = padding_i + dim + dim;
		min = v[seek];
		max = v[seek+1];
		seek = seek + d + d + 1;
		while(seek < v.size() && v[seek]<=1000000){
			if(min > v[seek]) min = v[seek];
			if(max < v[seek+1]) max = v[seek+1];
			seek = seek + d + d + 1;
		}
		v[dim+dim+1] = min;
		v[dim+dim+2] = max;
		dim++;
	}
	return;
}

int volume_min_internal(vector<int>& buff, int ptr1, int ptr2, int seek, vector<int>& newbuff, vector<int>& anotherbuff){

	if(seek == ptr1 || seek == ptr2) return 0;
	int v1 = volume_mbr(newbuff);
	int i = padding_i;
	while(i<newbuff.size()){
		if(newbuff[i]>1000000){
			for(int j=0;j<=d+d;j++){
				newbuff[i+j] = buff[seek+j];
			}
			break;
		}
		i += d+d+1;
	}
	update_mbr_internal(newbuff);
	int v2 = volume_mbr(newbuff);
	int diff1, diff2;
	diff1 = v2 - v1;
	v1 = volume_mbr(anotherbuff);
	i = padding_i;
	while(i<anotherbuff.size()){
		if(anotherbuff[i]>1000000){
			for(int j=0;j<=d+d;j++){
				anotherbuff[i+j] = buff[seek+j];
			}
			break;
		}
		i += d+d+1;
	}
	update_mbr_internal(anotherbuff);
	v2 = volume_mbr(anotherbuff);
	diff2 = v2 - v1;
	if(diff1 > diff2) return 2;
	return 1;
}

void update_parent_mbr(int node, int parent){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	vector<int> nodebuff, parentbuff;
	sprintf(file, "node%d.bin", node);
	int res = file_to_buff(file, nodebuff);
	sprintf(file, "node%d.bin", parent);
	res = file_to_buff(file, parentbuff);
	int i = parentbuff.size()-1, flag;
	while(i>=padding_i){
		flag = 0;
		if(parentbuff[i]==node){
			//cout << "update i, node, parent, ele - " << i << "," << node << "," << parent << "," << parentbuff[i] << endl;
			for(int j=0;j<d+d;j++){
				parentbuff[i-j-1] = nodebuff[d+d-j];
				//cout << nodebuff[d+d-j] << endl;
			}
			flag = 1;
			break;
		} else {
			i = i - d - d - 1;
		}
	}
	sprintf(file, "node%d.bin", parent);
	res = buff_to_file(file,parentbuff);
	if(flag==0){
		cout << "Error. No such child.." << endl;
		exit(0);
	}
	return;
}

void update_parent(int child, stack<int>& parent, int sibling){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	
	int p = parent.top();
	update_parent_mbr(child, p);
	//cout << "updating parent " << p << ", child, sib - " << child << "," << sibling << endl;
	int res;
	parent.pop();
	vector<int> buff, newbuff;

	sprintf(file, "node%d.bin", p);
	res = file_to_buff(file, buff);
	sprintf(file, "node%d.bin", sibling);
	res = file_to_buff(file, newbuff);
	
	int pos = padding_i;
	while(pos<buff.size()){
		if(buff[pos]>1000000) break;
		else pos = pos+d+d+1;
	}
	for(int i=0;i<d+d;i++){
		buff.insert(buff.begin()+pos, newbuff[i+1]);
		pos++;
	}
	buff.insert(buff.begin()+pos, sibling);


	if(buff[buff.size()-d-d-1] > 1000000){
		buff.resize(buff.size()-d-d-1);
		update_mbr_internal(buff);
		sprintf(file, "node%d.bin", p);
		res = buff_to_file(file, buff);
		if(p!=root) update_parent_mbr(p,parent.top());
	} else {
		//split
		newbuff.clear();
		sibling = new_node(0);
		sprintf(file, "node%d.bin", sibling);
		res = file_to_buff(file, newbuff);
		int max = -2000000;
		int ptr1, ptr2;
		//find two farthest rectangles
		for(int i=padding_i+2*d+1;i<buff.size();i=i+2*d+1){
			for(int j=padding_i;j<i;j=j+2*d+1){
				for(int dim=0;dim<d;dim++){
					int temp = maximum(buff[i+dim+dim],buff[j+dim+dim]) - minimum(buff[i+dim+dim+1],buff[j+dim+dim+1]);
					if(temp>max){
						max = temp;
						ptr1 = i;
						ptr2 = j;						
					}
				}
			}
		}

		//cout << "ptr1,ptr2 - " << ptr1 << "," << ptr2 << endl;

		vector<int> anotherbuff;
		res = new_buff(anotherbuff, 0);
		for(int i=0;i<=d+d;i++){
			anotherbuff[padding_i+i] = buff[ptr1+i];
			newbuff[padding_i+i] = buff[ptr2+i];
		}
		int i;
		max = (nodeCapacity)/2 + 1;
		int n1 = 1, n2 = 1;
		int seek = padding_i;
		
		//cout << "Putting other points" << endl;
		while(seek<buff.size()){

			//cout << "In while" << endl;
			if(seek == ptr1 || seek == ptr2){
				seek = seek + d + d + 1;
				continue;
			}
			if(n1>=max){	//
				i = padding_i;
				while(i<anotherbuff.size()){
					if(anotherbuff[i]>1000000){
						for(int j=0;j<=d+d;j++){
							anotherbuff[i+j] = buff[seek+j];
						}
						break;
					}
					i += d+d+1;
				}
				update_mbr_internal(anotherbuff);
				n2++;
				res = 0;
			} else if(n2>=max) {
				i = padding_i;
				while(i<newbuff.size()){
					if(newbuff[i]>1000000){
						for(int j=0;j<=d+d;j++){
							newbuff[i+j] = buff[seek+j];
						}
						break;
					}
					i += d+d+1;
				}
				update_mbr_internal(newbuff);
				n1++;
				res = 0;
			} else {
				res = volume_min_internal(buff, ptr1, ptr2, seek, newbuff, anotherbuff);
			}

			//cout << "ptr - " << seek << endl;
			//cout << "res - " << res << endl;
			seek = seek + d + d + 1;

			if(res == 0) continue;	//Point same as 1 or 2
			else if(res == 1){		//Put with 1, remove from 2
				i = padding_i;
				while(i<=anotherbuff.size()){
					//cout << "i - " << i << endl;
					if(i==anotherbuff.size() || anotherbuff[i]>1000000){
						for(int j=0;j<=d+d;j++){
							anotherbuff[i-j-1] = 1000001;
						}
						break;
					}
					i += d+d+1;
				}
				update_mbr_internal(anotherbuff);
				n1++;
			} else {				//Put with 2, remove from 1
				i = padding_i;
				while(i<=newbuff.size()){
					if(newbuff[i]>1000000 || i==newbuff.size()){
						for(int j=0;j<=d+d;j++){
							newbuff[i-j-1] = 1000001;
						}
						break;
					}
					i += d+d+1;
				}
				update_mbr_internal(newbuff);
				n2++;
			}

		}	//end of while.
		
		//cout << "Done.." << endl;
		buff.clear();
		
		sprintf(file, "node%d.bin", sibling);
		res = buff_to_file(file, newbuff);
		//newbuff.clear();
		sprintf(file, "node%d.bin", p);
		res = buff_to_file(file, anotherbuff);
		//anotherbuff.clear();
		if(p==root){	//new root.
			root = new_node(0);
			sprintf(file, "node%d.bin", root);
			res = file_to_buff(file, buff);
			int j;
			for(j=0;j<2*d;j++){
				buff[padding_i+j] = anotherbuff[j+1];
			}
			buff[padding_i+j] = p;
			j++;
			for(int k=0;k<2*d;k++){
				buff[padding_i+j] = newbuff[k+1];
				j++;
			}
			buff[padding_i+j] = sibling;
			update_mbr_internal(buff);
			res = buff_to_file(file, buff);
		} else {	//has parent.
			//cout << "Need to update parent.. " << p << "," << parent.top() << endl;
			update_parent(p, parent, sibling);
		}

	} //end of split

	return;

}

void insert(int node, vector<int> point, string object, stack<int>& parent){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	/*cout << "Inserting (";
	for (int i = 0; i < point.size(); i++)
	{
		cout << point[i] << ",";
	}
	cout << "), " << object << " in node" << node << endl;*/
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){ 	//Leaf Node
		//cout << "Leaf node " << node << endl;
		seek = padding;
		while(1){
			
			if(buff[seek]==point[0]){
				int i;
				for(i=1;i<point.size();i++){
					if(buff[seek+i]==point[i]);
					else break;
				}
				//cout << "i - " << i << endl;
				if(i==point.size()) break; //Point already present.
			}

			if(seek >= buff.size() || buff[seek]>1000000){
				int i;
				for(i=0;i<point.size();i++){
					buff.insert(buff.begin()+seek+i, point[i]);
				}
				buff.insert(buff.begin()+seek+i, objects);
				sprintf(file, "obj%d.bin", objects);
				objects++;
				ofstream file2;
				file2.open(file, ios::out | ios::binary);
				file2 << object;
				file2 << "\n";
				file2.close();
				if(buff[buff.size()-d-1] > 1000000){
					buff.resize(buff.size()-d-1);
					update_mbr(buff);
					sprintf(file, "node%d.bin", node);
					res = buff_to_file(file, buff);
					if(node!=root) update_parent_mbr(node,parent.top());
				} else {
					//Split.
					//cout << "Need to split " << node << endl;
					int sibling = new_node(1);
					int ptr1, ptr2;
					int j, k, dim =0;	//find 2 farthest points
					int max = 0;
					while(dim < d){
						for(j = padding+dim+d+1; j<buff.size(); j=j+d+1){
							for(k=padding+dim; k<j; k = k+d+1){
								if(abs(buff[j] - buff[k])>max){
									max = abs(buff[j] - buff[k]);
									ptr1 = j-dim;
									ptr2 = k-dim;
								}
							}
						}
						dim++;
					}
					//cout << "Points chosen - " << ptr1 << "," << ptr2 << endl;
					vector<int> newbuff, anotherbuff;
					res = new_buff(anotherbuff, 1);
					sprintf(file, "node%d.bin", sibling);
					res = file_to_buff(file, newbuff);
					for(j=0;j<=d;j++){	//Insert 2 farthest points
						anotherbuff[padding+j] = buff[ptr1+j];
						newbuff[padding+j] = buff[ptr2+j];
					}
					max = (leafCapacity)/2 + 1;
					int n1 = 1, n2 = 1;
					seek = padding;
					
					//cout << "Putting other points" << endl;
					while(seek<buff.size()){

						//cout << "In while" << endl;
						if(seek == ptr1 || seek == ptr2){
							seek = seek + d + 1;
							continue;
						}
						if(n1>=max){	//
							i = padding;
							while(i<anotherbuff.size()){
								if(anotherbuff[i]>1000000){
									for(int j=0;j<=d;j++){
										anotherbuff[i+j] = buff[seek+j];
									}
									break;
								}
								i += d+1;
							}
							update_mbr(anotherbuff);
							n2++;
							res = 0;
						} else if(n2>=max) {
							i = padding;
							while(i<newbuff.size()){
								if(newbuff[i]>1000000){
									for(int j=0;j<=d;j++){
										newbuff[i+j] = buff[seek+j];
									}
									break;
								}
								i += d+1;
							}
							update_mbr(newbuff);
							n1++;
							res = 0;
						} else {
							res = volume_min(buff, ptr1, ptr2, seek, newbuff, anotherbuff);
						}

						//cout << "ptr - " << seek << endl;
						//cout << "res - " << res << endl;
						seek = seek + d + 1;

						if(res == 0) continue;	//Point same as 1 or 2
						else if(res == 1){		//Put with 1, remove from 2
							i = padding;
							while(i<=anotherbuff.size()){
								//cout << "i - " << i << endl;
								if(i==anotherbuff.size() || anotherbuff[i]>1000000){
									for(int j=0;j<=d;j++){
										anotherbuff[i-j-1] = 1000001;
									}
									break;
								}
								i += d+1;
							}
							update_mbr(anotherbuff);
							n1++;
						} else {				//Put with 2, remove from 1
							i = padding;
							while(i<=newbuff.size()){
								if(newbuff[i]>1000000 || i==newbuff.size()){
									for(int j=0;j<=d;j++){
										newbuff[i-j-1] = 1000001;
									}
									break;
								}
								i += d+1;
							}
							update_mbr(newbuff);
							n2++;
						}

					}	//end of while.
					
					//cout << "Done.." << endl;
					buff.clear();
					sprintf(file, "node%d.bin", sibling);
					res = buff_to_file(file, newbuff);
					//newbuff.clear();
					sprintf(file, "node%d.bin", node);
					res = buff_to_file(file, anotherbuff);
					//anotherbuff.clear();
					if(node==root){	//new root.
						//cout << padding_i << "," << nodeCapacity << endl;
						root = new_node(0);
						sprintf(file, "node%d.bin", root);
						res = file_to_buff(file, buff);
						int j;
						for(j=0;j<2*d;j++){
							buff[padding_i+j] = anotherbuff[j+1];
						}
						buff[padding_i+j] = node;
						j++;
						for(int k=0;k<2*d;k++){
							buff[padding_i+j] = newbuff[k+1];
							j++;
						}
						buff[padding_i+j] = sibling;
						update_mbr_internal(buff);
						res = buff_to_file(file, buff);
					} else {	//has parent.
						//cout << "Need to update parent.. " << node << "," << parent.top() << endl;
						update_parent(node, parent, sibling);
					}

				}	//end of else(split)

				break;

			} else {
				seek = seek+d+1;
			} 
		} //end of while(1)

	} else {		//Internal Node
		//cout << "Internal node " << node << endl;
		parent.push(node);
		int ptr, v1, v2, diff, min = -1, temp;
		for(int pos=0;pos<d;pos++){
			if(buff[pos+pos+1]>point[pos]) buff[pos+pos+1] = point[pos];
			else if(buff[pos+pos+2]<point[pos]) buff[pos+pos+2] = point[pos];
		}
		res = buff_to_file(file, buff);
		int i = padding_i;
		int dim = 0;
		while(i<buff.size()){
			if(buff[i]>1000000) break;
			v1 = 1;
			while(dim<d){
				v1 *= buff[i+dim+dim+1] - buff[i+dim+dim];
				dim++;
			}
			dim = 0;
			v2 = 1;
			while(dim<d){
				int l1 = buff[i+dim+dim+1];
				int l2 = buff[i+dim+dim];
				if(point[dim]>l1) l1 = point[dim];
				if(point[dim]<l2) l2 = point[dim];
				v2 *= l1 - l2;
				dim++;
			}
			if(min==-1){
				min = v2-v1;
				ptr = i;
			}
			else {
				temp = v2-v1;
				if(temp<min){
					min = temp;
					ptr = i;
				}
			}
			i = i+d+d+1;
			//cout << "i - " << i << endl;
		}
		//cout << "ptr" << ptr+d+d << endl;
		insert(buff[ptr+d+d], point, object, parent);

	}	//End of internal node

	return;
}

void search(int node, vector<int> point, vector<int>& result){
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	/*cout << "Searching (";
	for (int i = 0; i < point.size(); i++)
	{
		cout << point[i] << ",";
	}
	cout << "), " << " in node" << node << endl;*/
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){ 	//Leaf Node
		seek = padding;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			for(int dim=0;dim<d;dim++){
				//cout << "dim,seek,buff - " << dim << "," << seek << "," << buff[seek+dim] << endl;
				if(buff[seek+dim]!=point[dim]){
					break;					
				} else {
					if(dim == d-1){
						result.push_back(1);
						return;
					}
				}
			}
			seek = seek+d+1;
		}
		result.push_back(0);
		return;
	} else {
		seek = padding_i;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			int flag = 0;
			for(int dim=0;dim<d;dim++){
				if(buff[seek+dim+dim]>point[dim] || buff[seek+dim+dim+1]<point[dim]){
					flag = 1;
					break;
				}
			}
			if(flag==0) search(buff[seek+d+d],point,result);
			seek = seek + d + d + 1;
		}
	}	
}

void range_search(int node, vector<int> pt, int range, list<string>& result){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	/*cout << "Searching (";
	for (int i = 0; i < pt.size(); i++)
	{
		cout << pt[i] << ",";
	}
	cout << "), r - " << range << " in node" << node << endl;*/
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){ 	//Leaf Node
		seek = padding;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			double dist = distance(pt,buff,seek);
			if((double)distance(pt,buff,seek)<=(double)range/1000000) add_object(result,buff[seek+d]);
			seek = seek+d+1;
		}
		return;
	} else {
		seek = padding_i;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			int flag = 0;
			flag = check_intersection(pt, range, buff, seek);
			if(flag==1) range_search(buff[seek+d+d],pt,range,result);
			seek = seek + d + d + 1;
		}
	}		
}

void knn_search(int node, vector<int> pt, int k, list<string>& result,myqueue *queue){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	/*cout << "Searching (";
	for (int i = 0; i < pt.size(); i++)
	{
		cout << pt[i] << ",";
	}
	cout << "), k - " << k << " in node" << node << endl;*/
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){ 	//Leaf Node
		seek = padding;
		while(seek<buff.size()){			
			if(buff[seek]>1000000) break;
			element *n = new element;
			n->type = 1;
			n->data = buff[seek+d];
			n->dist = distance(pt,buff,seek);
			queue->push(n);
			seek = seek+d+1;
		}
	} else {
		seek = padding_i;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			element *n = new element;
			n->type = 0;
			n->data = buff[seek+d+d];
			n->dist = dist_rect(pt,buff,seek);
			queue->push(n);
			seek = seek + d + d + 1;
		}
	}
	while(!queue->empty()){
		if(result.size()>=k) break;
		element *n = queue->top();
		queue->pop();
		if(n->type==1){
			add_object(result, n->data);			
		} else {
			knn_search(n->data, pt, k, result, queue);
			break;
		}
	}

}

void window_search(int node, vector<int>& mbr, list<string>& result){
	
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	/*cout << "Searching (";
	for (int i = 0; i < mbr.size(); i++)
	{
		cout << mbr[i] << ",";
	}
	cout << "), " << " in node" << node << endl;*/
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){ 	//Leaf Node
		seek = padding;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			for(int dim=0;dim<d;dim++){
				//cout << "dim,seek,buff - " << dim << "," << seek << "," << buff[seek+dim] << endl;
				if(buff[seek+dim]<=mbr[dim] || buff[seek+dim]>=mbr[dim+d]){
					break;					
				} else {
					if(dim == d-1){
						add_object(result, buff[seek+d]);
					}
				}
			}
			seek = seek+d+1;
		}
		return;
	} else {
		seek = padding_i;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			int flag = 0;
			for(int dim=0;dim<d;dim++){
				if(buff[seek+dim+dim]>mbr[dim+d] || mbr[dim]>buff[seek+dim+dim+1]){
					flag = 1;
					break;
				}
			}
			if(flag==0) window_search(buff[seek+d+d],mbr,result);
			seek = seek + d + d + 1;
		}
	}		
}

void query(string query_line, int flag){
	//if(flag>0)
	string args[2*d+1];
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
		vector<int> pt;
		for(i=1;i<=d;i++){
			pt.push_back((int) (1000000 * atof(args[i].c_str())));
		}
		stack<int> parent;
		start = clock();
		insert(root,pt,args[i],parent);
		end = clock();
		if(flag==1) insert_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		//if(flag>0)
		/*cout << "Inserted: (";
		for(i=0;i<d;i++) cout << args[i+1] << ",";
		cout << ") " << args[i+1] << endl;*/
	} else if(query_type==1){
		vector<int> pt;
		for(i=1;i<=d;i++){
			pt.push_back((int) (1000000 * atof(args[i].c_str())));
		}
		vector<int> result;
		start = clock();
		search(root, pt, result);
		end = clock();
		int flag=0;
		for(i=0;i<result.size();i++) if(result[i]==1) flag=1;
		search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*if(flag==0){
			cout << "Not found: (";					
			for(i=0;i<d;i++) cout << args[i+1] << ",";
			cout << ") " << endl;
		}
		else{
			cout << "Found (";
			for(i=0;i<d;i++) cout << args[i+1] << ",";
			cout << ") " << endl;
		}*/
	} else if(query_type==2){
		vector<int> pt;
		for(i=1;i<=d;i++){
			pt.push_back((int) (1000000 * atof(args[i].c_str())));
		}
		int range = (int) (1000000 * atof(args[i].c_str()));
		list<string> res;
		start = clock();
		range_search(root, pt, range, res);
		end = clock();
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*for(list<string>::iterator it = res.begin(); it!=res.end();++it){
			cout << "(" << (*it) << ")" << endl;
		}*/
	} else if(query_type==3){
		vector<int> pt;
		for(i=1;i<=d;i++){
			pt.push_back((int) (1000000 * atof(args[i].c_str())));
		}
		int k = (int) (atof(args[i].c_str()));
		myqueue queue;
		list<string> res;
		start = clock();
		knn_search(root, pt, k, res, &queue);
		end = clock();
		knn_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*for(list<string>::iterator it = res.begin(); it!=res.end();++it){
			cout << "(" << (*it) << ")" << endl;
		}*/
	} else if(query_type==4){
		vector<int> mbr;
		for(i=1;i<=d+d;i++){
			mbr.push_back((int) (1000000 * atof(args[i].c_str())));
		}
		list<string> result;
		start = clock();
		window_search(root, mbr, result);
		end = clock();
		window_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*for(list<string>::iterator it = result.begin(); it!=result.end();++it){
			cout << "(" << (*it) << ")" << endl;
		}*/
	}
	//if(flag>0)
	cout << "\n" ;
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
	config_file.open("rtree.config");
	config_file >> pageSize;
	config_file >> d;
	leafCapacity = ((pageSize/4)-(2*d+1))/(d+1);
	padding = (pageSize/4)-(d+1)*leafCapacity;
	nodeCapacity = (pageSize/(4*(2*d+1)))-1;
	padding_i = (pageSize/4)-(2*d+1)*nodeCapacity;
	char* file;
	file = (char*) malloc (15*sizeof(char));	
	root = new_node(1);
	ifstream file1("assgn4_r_data.txt");
	//ifstream file1("d.txt");
	//int n=0;
	if(file1.is_open()){
		while(getline(file1, query_line)){
			//n++;
			//cout << "n: " << n << endl;
			query("0	" + query_line, 0);
		}
	} else cout << "Error opening file1.." << endl;
	ifstream file3("aassgn4_r_querysample.txt");
	//ifstream file3("q.txt");
	if(file3.is_open()){
		while(getline(file3, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening file.." <<endl;
	
	cout << "Tree rooted at node " << root << endl;	
	
	ofstream output;
	double sum, mean, sq_sum, st_dev;
	output.open("rtree.out");
	output << "All stats in milliseconds.\n\nInsertion stats:\n";
	output << "min - " << *min_element(insert_times.begin(), insert_times.end()) << "\n";
	output << "max - " << *max_element(insert_times.begin(), insert_times.end()) << "\n";
	sum = std::accumulate(insert_times.begin(), insert_times.end(), 0.0);
	mean = sum / insert_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(insert_times, mean) << "\n";
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
	output << "\nWindow Search stats:\n";
	output << "min - " << *min_element(window_search_times.begin(), window_search_times.end()) << "\n";
	output << "max - " << *max_element(window_search_times.begin(), window_search_times.end()) << "\n";
	sum = std::accumulate(window_search_times.begin(), window_search_times.end(), 0.0);
	mean = sum / window_search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(window_search_times, mean) << "\n";
	output.close();

}
