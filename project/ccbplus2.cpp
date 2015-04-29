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

int pageSize, S;

int root;
int cnt = 1;
int objects = 1;
int leafCapacity;
int nodeCapacity;
int cacheCapacity;
int cacheIndex;
int padding, padding_l;
int cachehits = 0;
clock_t start, end;
vector<double> insert_times;
vector<double> search_times;
vector<double> range_search_times;
vector<double> knn_search_times;
vector<double> window_search_times;

//0 implies null.
	
/**
Internal nodes:
isleaf - 4B, key1 - 4B, rp1 - 4B, key2 - 4B, rp2 - 4B, C1, C2........
Ci is of form - ci,ki,pi,ki' > popularity count,ki-ki' range in pi leaf
Leaf Node:
isleaf - 4B, prev - 4B, next - 4B, count, point1 - 4B, point2 - 4B, point3 - 4B, ........
**/

int new_node(int node_type){
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", cnt++);
	ofstream file1;
	file1.open(file, ios::out | ios::binary);
	int data = 1;
	for(int i=1;i<=(pageSize/4);i++){
		if(i==1) data = node_type;
		else if(i<=padding_l && node_type==1) data = 0;
		else if(i<=padding && node_type==0) data = 0;
		else data = 1000001;
		file1.write((char*)&data,sizeof(data));
	}
	file1.close();
	return (cnt-1);
}

int buff_to_file(char *file, vector<int>& buff){
	// cout << file << endl;
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

void add_object(list<string> *queue, int objId){
	ifstream objFile;
	char *temp;
	temp = (char *) malloc (15*sizeof(char));
	sprintf(temp, "obj%d.bin", objId);
	objFile.open(temp);
	string obj;
	objFile >> obj;
	queue->push_back(obj);
}

void updateCaches(int count, int node, int k1, int k2, int parent_node, stack<int>& parent){
	// cout << "updating cache" << endl;
	int res;
	char *file = (char *) malloc (sizeof(char)*15);
	sprintf(file, "node%d.bin", parent_node);
	vector<int> buff;
	res = file_to_buff(file,buff);
	int flag = 0;
	int write = -1;
	int temp = -1;
	int index;
	for(int i=cacheIndex+2;i<buff.size();i+=2){
		if(buff[i]==node){
				flag = 1;
				buff[i-2] = count;
				write = 1;
				break;
		} else if(buff[i-2]>1000000){
				flag = -1;
				index = i;
				write = 1;
				break;
		} else if(temp==-1 || buff[i-2]<temp){
				temp = buff[i-2];
				flag = -1;
				write = 1;
				index = i;
		}
	}
	if(flag==-1){
		buff[index-2] = count;
		buff[index-1] = k1;
		buff[index] = node;
		buff[index+1] = k2;
	}
	if(write==1) res = buff_to_file(file,buff);
	else return;
	if(parent_node==root) return;
	parent_node = parent.top();
	parent.pop();
	updateCaches(count, node, k1, k2, parent_node, parent);

}

void update_parent(int key, int sibling, stack<int>& parent){


	// cout << "Updaada" << endl;
	char* file;
	file = (char*) malloc (15*sizeof(char));
	
	int p = parent.top();
	// cout << "updating parent " << p << ", k, sib - " << key << "," << sibling << endl;
	int res, split;
	parent.pop();
	vector<int> newbuff;
	sprintf(file, "node%d.bin", p);
	res = file_to_buff(file, newbuff);
	bool inserted = false;
	for(int i=padding+1;i<cacheIndex;i+=2){
		if(newbuff[i]>key){
			inserted = true;
			newbuff.insert(newbuff.begin()+i, key);
			newbuff.insert(newbuff.begin()+i+1, sibling);
			break;
		}
	}
	if(!inserted){
		newbuff.insert(newbuff.begin()+cacheIndex, key);
		newbuff.insert(newbuff.begin()+cacheIndex+1, sibling);
	}

	//for(int i=0;i<newbuff.size();i++) cout << newbuff[i] << endl;
	if(newbuff[cacheIndex]>1000000){
		newbuff.erase(newbuff.begin()+cacheIndex,newbuff.begin()+cacheIndex+2);
		res = buff_to_file(file, newbuff);
	} else {	//split parent too.
		sibling = new_node(0); //parent's sibling
		split = padding + nodeCapacity;
		if(nodeCapacity%2==0) split++;
		key = newbuff[split];
		newbuff[split] = 1000001;
		vector<int> anotherbuff;
		sprintf(file, "node%d.bin", sibling);
		res = file_to_buff(file, anotherbuff);
		int j=padding;
		for(int i=cacheIndex;i<newbuff.size();i++){
			anotherbuff[i] = newbuff[i+2];
		}
		for(int i=split+1;i<cacheIndex+2;i++){
			anotherbuff[j] = newbuff[i];
			newbuff[i] = 1000001;
			j++;
		}
		res = buff_to_file(file, anotherbuff);
		sprintf(file, "node%d.bin", p);
		newbuff.erase(newbuff.begin()+cacheIndex,newbuff.begin()+cacheIndex+2);
		res = buff_to_file(file, newbuff);
		if(p==root){
			root = new_node(0);
			newbuff.clear();
			sprintf(file, "node%d.bin", root);
			res = file_to_buff(file, newbuff);
			newbuff[padding] = p;
			newbuff[padding+1] = key;
			newbuff[padding+2] = sibling;
			res = buff_to_file(file, newbuff);
		} else { 
			update_parent(key, sibling, parent);
		}
	}							
}

void insert(int node, int val, string object, stack<int>& parent, stack<int>& parent2){
	/*if(node==0){
		cout << "Reached node 0 in insert" << endl;
		return;
	}*/
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	//cout << "Inserting " << val << ", " << object << " in node" << node << endl;
	int fp, data;
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;

	if (buff[0]){	//leaf node.
		// cout << "Leaf node " << node << endl;
		seek = padding_l;	//Leaving 4 spaces. 1 for isleaf, 2 for prev, next, 1 count.
		while(1){
			// cout << "seek - " << seek << endl;
			if(seek >= buff.size() || buff[seek] > val){
				buff.insert(buff.begin()+seek, val);
				buff.insert(buff.begin()+seek+1, objects);
				sprintf(file, "obj%d.bin", objects);
				objects++;
				ofstream file2;
				file2.open(file, ios::out | ios::binary);
				file2 << object;
				file2 << "\n";
				file2.close();
				if(buff[buff.size()-2]>1000000){
					buff.resize(buff.size()-2);
					sprintf(file, "node%d.bin", node);
					buff[3] = buff[3]+1;
					int count = buff[3];
					int min = buff[padding_l], max;
					int c = buff.size()-2;
					while(buff[c]>1000000) c -= 2;
					max = buff[c];
					res = buff_to_file(file,buff);
					buff.clear();
					if(!parent2.empty()){
						int parent_node = parent2.top();
						parent2.pop();
						sprintf(file, "node%d.bin", parent_node);
						res = file_to_buff(file,buff);
						// cout << endl << min << ", max-" << max << endl;
						int temp = count;
						int flag = 0, index = 0, write = -1;
						for(int i=cacheIndex+2;i<buff.size();i+=4){
							if(buff[i]==node){
								flag = -1;
								temp = count;
								write = 1;
								index = i;
								break;
							} else if(buff[i-2]>1000000){
								flag = -1;
								index = i;
								write = 1;
								break;
							} else if(temp==-1 || buff[i-2]<temp){
								temp = buff[i-2];
								flag = -1;
								write = 1;
								index = i;
							}
						}
						if(flag==-1){
							buff[index-2] = count;
							buff[index-1] = min;
							buff[index] = node;
							buff[index+1] = max;
						}
						if(write==1){
							res = buff_to_file(file,buff);
							if(!parent2.empty()){
								parent_node = parent2.top();
								parent2.pop();
								updateCaches(count, node, min, max, parent_node, parent2);
							}
						}
					}
				} else {
					//Split.
					// cout << "Need to split node " << node << endl;
					// exit(0);
					int sibling = new_node(1);
					int split = leafCapacity+padding_l;
					if(leafCapacity%2==1) split++;
					vector<int> newbuff;
					sprintf(file, "node%d.bin", sibling);
					res = file_to_buff(file, newbuff);
					newbuff[1] = node;
					newbuff[2] = buff[2];
					buff[2] = sibling;
					if(newbuff[2]!=0){
						char * temp;
						temp = (char *) malloc (15*sizeof(char));
						sprintf(temp, "node%d.bin", newbuff[2]);
						vector<int> v;
						res = file_to_buff(temp, v);
						v[1] = sibling;
						res = buff_to_file(temp, v);						
					}
					int j=padding_l; 
					for(int i=split;i<buff.size();i++){
						newbuff[j] = buff[i];
						buff[i] = 1000001;
						j++;
					}
					newbuff[3] = buff[3] + 1;
					int key = newbuff[padding_l];
					sprintf(file, "node%d.bin", sibling);
					res = buff_to_file(file, newbuff);
					sprintf(file, "node%d.bin", node);
					buff.resize(buff.size()-2);
					buff[3] = buff[3]+1;
					int count = buff[3];
					int min = buff[padding_l], max;
					int c = buff.size()-2;
					while(buff[c]>1000000) c -= 2;
					max = buff[c];
					res = buff_to_file(file,buff);
					if(!parent2.empty()){
						buff.clear();
						int parent_node = parent2.top();
						parent2.pop();
						sprintf(file, "node%d.bin", parent_node);
						res = file_to_buff(file,buff);
						
						int temp = count;
						int flag = 0, index = 0, write = -1;
						for(int i=cacheIndex+2;i<buff.size();i+=4){
							if(buff[i]==node){
								flag = -1;
								temp = count;
								write = 1;
								index = i;
								break;
							} else if(buff[i-2]>1000000){
								flag = -1;
								index = i;
								write = 1;
								break;
							} else if(temp==-1 || buff[i-2]<temp){
								temp = buff[i-2];
								flag = -1;
								write = 1;
								index = i;
							}
						}
						if(flag==-1){
							buff[index-2] = count;
							buff[index-1] = min;
							buff[index] = node;
							buff[index+1] = max;
						}
						if(write==1){
							res = buff_to_file(file,buff);
							if(!parent2.empty()){
								parent_node = parent2.top();
								parent2.pop();
								updateCaches(count, node, min, max, parent_node, parent2);
							}
						}
					}
					if(node==root){
						newbuff.clear();
						root = new_node(0);
						sprintf(file, "node%d.bin", root);
						res = file_to_buff(file, newbuff);
						newbuff[padding] = node;
						newbuff[padding+1] = key;
						newbuff[padding+2] = sibling;
						res = buff_to_file(file, newbuff);							
					} else {
						update_parent(key, sibling, parent);
					}
				}
				break;
			} 
			else if(buff[seek]==val){
				buff[3] = buff[3]+1;
				int count = buff[3];
				int min = buff[padding_l], max;
				int c = buff.size()-2;
				while(buff[c]>1000000) c -= 2;
				max = buff[c];
				res = buff_to_file(file,buff);
				if(!parent2.empty()){
					buff.clear();
					int parent_node = parent2.top();
					parent2.pop();
					sprintf(file, "node%d.bin", parent_node);
					res = file_to_buff(file,buff);
					
					int temp = count;
					int flag = 0, index = 0, write = -1;
					for(int i=cacheIndex+2;i<buff.size();i+=4){
						if(buff[i]==node){
							flag = -1;
							temp = count;
							index = i;
							write = 1;
							break;
						} else if(buff[i-2]>1000000){
							flag = -1;
							index = i;
							write = 1;
							break;
						} else if(temp==-1 || buff[i-2]<temp){
							temp = buff[i-2];
							flag = -1;
							write = 1;
							index = i;
						}
					}
					if(flag==-1){
						buff[index-2] = count;
						buff[index-1] = min;
						buff[index] = node;
						buff[index+1] = max;
					}
					if(write==1){
						res = buff_to_file(file,buff);
						if(!parent2.empty()){
							parent_node = parent2.top();
							parent2.pop();
							updateCaches(count, node, min, max, parent_node, parent2);
						}
					}
				}
				return;	//Already present.
			}
			else {
				seek+=2;			
			}
		}
		/*sprintf(file, "node%d.bin", node);
		res = file_to_buff(file, buff);
		for(int i=0;i<buff.size();i++){
			cout << buff[i] << endl;
		}*/
	} else { //Internal node.
		// cout << "Internal node " << node << endl;
		/*for(int i=0;i<buff.size();i++){
			cout << buff[i] << endl;
		}*/
		seek = padding+1; //1 for isleaf, 1 for 1st child pointer.
		parent.push(node);
		parent2.push(node);
		/*for(int i=0;i<cacheCapacity;i++){
			if(buff[cacheIndex + i*4 + 1] <= val && buff[cacheIndex + i*4 + 3] > val){
				insert(buff[cacheIndex + i*4 + 2], val, object, parent, parent2);
				return;
			}
		}*/
		while(seek<cacheIndex){
			//cout << buff[seek] << endl;
			if(buff[seek]>1000000){
				insert(buff[seek-1], val, object, parent, parent2);
				break;
			} else {
				if(buff[seek]==val){
					insert(buff[seek+1], val, object, parent, parent2);
					break;
				} else if(buff[seek] > val){
					insert(buff[seek-1], val, object, parent, parent2);
					break;
				} else {
					seek+=2;
					if(seek>=cacheIndex){
						//cout << "last ele " << buff[seek-1] << endl;
						insert(buff[seek-1], val, object, parent, parent2);
						break;
					}
				}
			}
		}
		//cout << "seek " << seek << endl;
	}

}

int search(int node, int val, stack<int>& parent){

	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	// cout << "Searching " << val << ", " << " in node" << node << endl;
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file" << file << endl;
	int seek = 0;
	if(buff[0]==1){	//Leaf node
		seek = padding_l;
		buff[3] = buff[3]+1;
		int count = buff[3];
		int ans = 0;
		while(seek<buff.size()){
			if(buff[seek]==val) ans = 1;
			seek+=2;
		}
		res = buff_to_file(file,buff);
		buff.clear();
		if(!parent.empty()){
			int parent_node = parent.top();
			parent.pop();
			sprintf(file, "node%d.bin", parent_node);
			res = file_to_buff(file,buff);
			int min, max;
			for(int i=padding;i<cacheIndex;i+=2){
				if(buff[i]==node){
					if(i-1 < padding) min = 0;
					else min = buff[i-1];
					if(i+1 < cacheIndex) max = buff[i+1];
					else max = 1000001;
				}
			}
			int temp = count;
			int flag = 0, index = 0, write = -1;
			for(int i=cacheIndex+2;i<buff.size();i+=4){
				if(buff[i]==node){
					flag = -1;
					temp = count;
					write = 1;
					index = i;
					break;
				} else if(buff[i-2]>1000000){
					flag = -1;
					index = i;
					write = 1;
					break;
				} else if(temp==-1 || buff[i-2]<temp){
					temp = buff[i-2];
					flag = -1;
					write = 1;
					index = i;
				}
			}
			if(flag==-1){
				buff[index-2] = count;
				buff[index-1] = min;
				buff[index] = node;
				buff[index+1] = max;
			}
			if(write==1) res = buff_to_file(file,buff);
			else return ans;
			if(!parent.empty()){
				parent_node = parent.top();
				parent.pop();
				updateCaches(count, node, min, max, parent_node, parent);
			}
		}
		return ans;
	} else {	//Internal node
		parent.push(node);
		for(int i=0;i<cacheCapacity;i++){
			if(buff[cacheIndex + i*4 + 1] <= val && buff[cacheIndex + i*4 + 3] > val){
				cachehits++;
				return search(buff[cacheIndex + i*4 + 2], val, parent);
			}
		}
		seek = padding+1; //1 for isleaf, 1 for 1st child pointer.
		while(seek<cacheIndex){
			//cout << buff[seek] << endl;
			if(buff[seek]>1000000){
				return search(buff[seek-1], val, parent);
			} else {
				if(buff[seek]==val){
					return search(buff[seek+1], val, parent);
				} else if(buff[seek] > val){
					return search(buff[seek-1], val, parent);
				} else {
					seek+=2;
					if(seek>=cacheIndex){
						return search(buff[seek-1], val, parent);
					}
				}
			}
		}
	}
}

void window_search(int node, int low, int high, list<string> *queue, stack<int>& parent){
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	// cout << "Searching window " << low << ", " << high << " in node" << node << endl;
	vector<int> buff;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;
	if(buff[0]==1){	//Leaf node
		seek = padding_l;
		// cout << "Leaf " << node << endl;
		bool end = false;
		while(seek<buff.size()){
			if(buff[seek]>1000000) break;
			if(buff[seek]>=low && buff[seek]<=high)	add_object(queue, buff[seek+1]);
			else if(buff[seek]>high) end = true;
			seek+=2;
		}
		buff[3] = buff[3]+1;
		int count = buff[3];
		int next_node = buff[2];
		res = buff_to_file(file,buff);
		if(!parent.empty()){
			buff.clear();
			int parent_node = parent.top();
			parent.pop();
			sprintf(file, "node%d.bin", parent_node);
			res = file_to_buff(file,buff);
			int min, max;
			for(int i=padding;i<cacheIndex;i+=2){
				if(buff[i]==node){
					if(i-1 < padding) min = 0;
					else min = buff[i-1];
					if(i+1 < cacheIndex) max = buff[i+1];
					else max = 1000001;
				}
			}
			int temp = count;
			int flag = 0, index = 0, write = -1;
			for(int i=cacheIndex+2;i<buff.size();i+=4){
				if(buff[i]==node){
					flag = -1;
					temp = count;
					index = i;
					write = 1;
					break;
				} else if(buff[i-2]>1000000){
					flag = -1;
					index = i;
					write = 1;
					break;
				} else if(temp==-1 || buff[i-2]<temp){
					temp = buff[i-2];
					flag = -1;
					write = 1;
					index = i;
				}
			}
			if(flag==-1){
				buff[index-2] = count;
				buff[index-1] = min;
				buff[index] = node;
				buff[index+1] = max;
			}
			if(write==1){
				res = buff_to_file(file,buff);
				if(!parent.empty()){
					parent_node = parent.top();
					parent.pop();
					updateCaches(count, node, min, max, parent_node, parent);
				}
			}
		}
		if(end) return;
		if(next_node!=0){
			window_search(next_node, low, high, queue, parent);
		}
	} else {	//Internal node
		parent.push(node);
		for(int i=0;i<cacheCapacity;i++){
			if(buff[cacheIndex + i*4 + 1] <= low && buff[cacheIndex + i*4 + 3] > low){
				cachehits++;
				window_search(buff[cacheIndex + i*4 + 2], low, high, queue, parent);
				return;
			}
		}
		seek = padding+1; //1 for isleaf, 1 for 1st child pointer.
		while(seek<cacheIndex){
			//cout << buff[seek] << endl;
			if(buff[seek]>1000000){
				window_search(buff[seek-1], low, high, queue, parent);
				return;
			} else {
				if(buff[seek]==low){
					window_search(buff[seek+1], low, high, queue, parent);
					return;
				} else if(buff[seek] > low){
					window_search(buff[seek-1], low, high, queue, parent);
					return;
				} else {
					seek+=2;
					if(seek>=cacheIndex){
						window_search(buff[seek-1], low, high, queue, parent);
						return;
					}
				}
			}
		}
	}

}

void range_search(int node, int point, int range, list<string> *queue, stack<int>& parent){
	window_search(node, point-range, point+range, queue, parent);
}

void knn_search(int node, int point, int n, list<string> *queue, stack<int>& parent){
	char* file;
	file = (char*) malloc (15*sizeof(char));
	sprintf(file, "node%d.bin", node);
	// cout << "Searching for knn " << point << ", " << n << " in node" << node << endl;
	vector<int> buff, leftbuff, rightbuff;
	int left, right;
    int res = file_to_buff(file,buff);
    if(res==0) cout << "Error in opening file " << file << endl;
	int seek = 0;
	if(buff[0]==1){	//Leaf node
		seek = padding_l;
		while(seek<buff.size()){
			if(buff[seek]>1000000){
				left = seek-2;
				right = buff.size()+1;
				break;
			} else if(buff[seek]>point){
				left = seek-2;
				right = seek;
				break;
			} else if(buff[seek]==point){
				left = seek-2;
				right = seek+2;								
				break;
			}
			seek+=2;
			if(seek>buff.size()){
				left = seek-2;
				right = buff.size()+1;			
			}
		}// end of while

		if(left<padding_l){
			if(buff[1]!=0){
				sprintf(file, "node%d.bin", buff[1]);
				leftbuff.clear();
				res = file_to_buff(file,leftbuff);
				left = leftbuff.size()-2;
			} else {
				left = 0;
			}
		} else {
			sprintf(file, "node%d.bin", node);
			leftbuff.clear();			
			res = file_to_buff(file,leftbuff);
		}
		
		if(right>=buff.size()){
			if(buff[2]!=0){
				right = padding_l;
				sprintf(file, "node%d.bin", buff[2]);
				rightbuff.clear();
				res = file_to_buff(file,rightbuff);
			} else {
				right = 0;
			}
		} else{
			sprintf(file, "node%d.bin", node);
			rightbuff.clear();
			res = file_to_buff(file,rightbuff);	
		}	
		while(n > queue->size()){
			//cout << "left - " << left << ", right - " << right << ", leftbuff[1] - " << leftbuff[1] << ", rightbuff[2] - " << rightbuff[2] << endl;

			if(left == 0 && right==0) break;
			else if(left == 0){
				add_object(queue, rightbuff[right+1]);
				right += 2;
			} else if(right == 0){
				add_object(queue, leftbuff[left+1]);
				left -= 2;
			} else {
				if(abs(leftbuff[left]-point) < abs(rightbuff[right]-point)){
					add_object(queue, leftbuff[left+1]);
					left -= 2;
				} else {
					add_object(queue, rightbuff[right+1]);
					right += 2;
				}
			}
			if(left<padding_l){
				if(leftbuff[1]!=0){
					sprintf(file, "node%d.bin", leftbuff[1]);
					leftbuff.clear();
					res = file_to_buff(file,leftbuff);
					left = leftbuff.size()-2;
					while(leftbuff[left]>1000000){
						left -= 2;
					}
				} else {
					left = 0;
				}
			}
			
			if(right>=rightbuff.size() || rightbuff[right]>1000000){
				if(rightbuff[2]!=0){
					right = padding_l;
					sprintf(file, "node%d.bin", rightbuff[2]);
					rightbuff.clear();
					res = file_to_buff(file,rightbuff);
				} else {
					right = 0;
				}
			}
		}

		buff[3] = buff[3]+1;
		int count = buff[3];
		sprintf(file, "node%d.bin", node);
		res = buff_to_file(file,buff);
		if(!parent.empty()){
			buff.clear();
			int parent_node = parent.top();
			parent.pop();
			sprintf(file, "node%d.bin", parent_node);
			res = file_to_buff(file,buff);
			int min, max;
			for(int i=padding;i<cacheIndex;i+=2){
				if(buff[i]==node){
					if(i-1 < padding) min = 0;
					else min = buff[i-1];
					if(i+1 < cacheIndex) max = buff[i+1];
					else max = 1000001;
				}
			}
			int temp = count;
			int flag = 0, index = 0, write = -1;
			for(int i=cacheIndex+2;i<buff.size();i+=4){
				if(buff[i]==node){
					flag = -1;
					temp = count;
					write = 1;
					index = i;
					break;
				} else if(buff[i-2]>1000000){
					flag = -1;
					index = i;
					write = 1;
					break;
				} else if(temp==-1 || buff[i-2]<temp){
					temp = buff[i-2];
					flag = -1;
					write = 1;
					index = i;
				}
			}
			if(flag==-1){
				buff[index-2] = count;
				buff[index-1] = min;
				buff[index] = node;
				buff[index+1] = max;
			}
			if(write==1) res = buff_to_file(file,buff);
			else return;
			if(!parent.empty()){
				parent_node = parent.top();
				parent.pop();
				updateCaches(count, node, min, max, parent_node, parent);
			}
		}
		return;
	} else {	//Internal node
		parent.push(node);
		for(int i=0;i<cacheCapacity;i++){
			if(buff[cacheIndex + i*4 + 1] <= point && buff[cacheIndex + i*4 + 3] > point){
				cachehits++;
				knn_search(buff[cacheIndex + i*4 + 2], point, n, queue, parent);
				return;
			}
		}
		seek = padding+1; //1 for isleaf, 1 for 1st child pointer.
		while(seek<cacheIndex){
			//cout << buff[seek] << endl;
			if(buff[seek]>1000000){
				knn_search(buff[seek-1], point, n, queue, parent);
				return;
			} else {
				if(buff[seek]==point){
					knn_search(buff[seek+1], point, n, queue, parent);
					return;
				} else if(buff[seek] > point){
					knn_search(buff[seek-1], point, n, queue, parent);
					return;
				} else {
					seek+=2;
					if(seek>=cacheIndex){
						knn_search(buff[seek-1], point, n, queue, parent);
						return;
					}
				}
			}
		}
	}
}

void query(string query_line, int flag){
	//if(flag>0)
	string args[4];
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
		int arg1;
		arg1 = (int) (1000000 * atof(args[1].c_str()));
		stack<int> parent, parent2;
		start = clock();
		insert(root,arg1,args[2],parent, parent2);
		end = clock();
		if(flag==1) insert_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		//if(flag>0)
		// cout << "Inserted: (" << arg1 << "," << args[2] << ")" << endl;
	} else if(query_type==1){
		int res=0, arg1;
		arg1 = (int) (1000000 * atof(args[1].c_str()));
		stack<int> parent;
		start = clock();
		res = search(root, arg1, parent);
		end = clock();
		search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		// if(res==0) cout << "Not found: (" << args[1] << ")" << endl;
		// else cout << "Found (" << args[1] << ")" << endl;
	} else if(query_type==2){
		int arg1 = (int) (1000000 * atof(args[1].c_str()));;
		int arg2 = (int) (1000000 * atof(args[2].c_str()));;
		list<string> res;
		stack<int> parent;
		start = clock();
		range_search(root, arg1, arg2, &res, parent);
		end = clock();
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		// for(list<string>::iterator it = res.begin(); it!=res.end();++it){
		// 	cout << "(" << (*it) << ")" << endl;
		// }
	} else if(query_type==3){
		int arg1 = (int) (1000000 * atof(args[1].c_str()));;
		int arg2 = (int) (atof(args[2].c_str()));;
		list<string> res;
		stack<int> parent;
		start = clock();
		knn_search(root, arg1, arg2, &res, parent);
		end = clock();
		knn_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		// for(list<string>::iterator it = res.begin(); it!=res.end();++it){
		// 	cout << "(" << (*it) << ")" << endl;
		// }
	} else if(query_type==4){
		int arg1 = (int) (1000000 * atof(args[1].c_str()));;
		int arg2 = (int) (1000000 * atof(args[2].c_str()));;
		if(arg1 > arg2){
			int temp = arg1;
			arg1 = arg2;
			arg2 = temp;
		}
		list<string> res;
		stack<int> parent;
		start = clock();
		window_search(root, arg1, arg2, &res, parent);
		end = clock();
		window_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		// for(list<string>::iterator it = res.begin(); it!=res.end();++it){
		// 	cout << "(" << (*it) << ")" << endl;
		// }
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
	config_file.open("ccbplustree.config");
	config_file >> pageSize;
	double t;
	config_file >> t;
	S = pageSize * t;
	// cout << "S - " <<  S << endl;
	leafCapacity = (pageSize-16)/8;
	padding_l = (pageSize-(leafCapacity*8))/4;
	cacheCapacity = (pageSize-S)/16;
	nodeCapacity = (pageSize-(cacheCapacity*16)-8)/8;
	padding = (pageSize/4)-(cacheCapacity*4 + nodeCapacity*2 + 1);
	cacheIndex = (pageSize-cacheCapacity*16)/4;

	// cout << pageSize<< ","<< cacheIndex << "," << nodeCapacity << "," << leafCapacity << "," << cacheCapacity << "," << padding << "," << padding_l << endl;
	// exit(0);
	char* file;
	file = (char*) malloc (15*sizeof(char));	
	root = new_node(1);
	fstream file2;
	int data = 1;
	sprintf(file, "node%d.bin", root);
	file2.open(file, ios::in | ios::out | ios::binary);
	file2.seekp(0, ios::beg);
	file2.write((char*)(&data),sizeof(data)); data = 0;		//isleaf
	file2.write((char*)(&data),sizeof(data));				//prev
	file2.write((char*)(&data),sizeof(data));				//next
	file2.write((char*)(&data),sizeof(data));				//waste
	file2.close();
	//cout << "Reading file.." << endl;
	// ifstream file1("assgn3_bplus_data.txt");
	ifstream file1("d.txt");
	int n=0;
	if(file1.is_open()){
		while(getline(file1, query_line)){
			n++;
			cout << "data: " << n << endl;
			query("0	" + query_line, 0);
		}
	} else cout << "Error opening file1.." << endl;
	cout << "Tree rooted at node " << root << endl;		
	// exit(0);
	// ifstream file3("assgn3_bplus_querysample.txt");
	n=0;
	ifstream file3("q.txt");
	if(file3.is_open()){
		while(getline(file3, query_line)){
			n++;
			cout << "query: " << n << endl;
			query(query_line, 1);
		}
	} else cout << "Error opening file.." <<endl;
	
	ofstream output;
	double sum, mean, sq_sum, st_dev;
	output.open("ccbplus.out");
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
	output << "\nCache Hits: - " << cachehits << "\n";
	output.close();

	return 0;
}


