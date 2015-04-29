#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <list>
#include <cmath>
#include <vector>
#include <functional>
#include <queue>
#include <time.h>
#include <iomanip>
#include <numeric>
#include <algorithm>

using namespace std;

int dist_func = 1;
double matrix[2][2];

class Point{
	public:
		double x;
		double y;		
		int id;
		string obj;
	Point(double x_cod, double y_cod){
		x = x_cod;
		y = y_cod;
	}
	Point(double x_cod, double y_cod, int num, string str){
		x = x_cod;
		y = y_cod;
		id = num;
		obj = str;
	}
};

clock_t start, end;
vector<double> range_search_times;
vector<double> knn_search_times;
vector<double> rdistance_counts;
vector<double> kdistance_counts;

struct element{
	Point *p; //0 for rect, 1 for pt
	double dist;
};

struct MyComparator{
    bool operator() (element *n1, element *n2){
         return (n1->dist) < (n2->dist);
    }
};

typedef priority_queue<element*, vector<element*>, MyComparator> myqueue;

class ghTree;
int counter;

double mahalanobhis_distance(Point *p1, Point *p2){
	double a, b;
	double dx = abs(p1->x - p2->x);
	double dy = abs(p1->y - p2->y);
	a = dx*matrix[0][0] + dy*matrix[1][0];
	b = dx*matrix[0][1] + dy*matrix[1][1];	
	a = abs(dx*a + dy*b);
	return sqrt(a);
}

double euclidean_distance(Point *p1, Point *p2){
	double dx = p1->x - p2->x;
	double dy = p1->y - p2->y;
	return sqrt(dx*dx + dy*dy);
}

double distance(Point *p1, Point *p2){
	counter++;
	if(dist_func==1) return euclidean_distance(p1,p2);
	else return mahalanobhis_distance(p1,p2);
}

double maximum(double a, double b){
	return ((a>b) ? a : b);
}

class ghTree{

	public:
		Point *p1;
		Point *p2;
		vector<Point*> points;
		ghTree *left;
		ghTree *right;

		ghTree(){
			left = NULL;
			right = NULL;
			p1 = NULL;
			p2 = NULL;
		}

		void printtree(){
			if(p1!=NULL) cout << "\t(" << p1->x << "," << p1->y << ")\t(" << p2->x << "," << p2->y << ")" << endl;
			else{
				for(int i=0;i<points.size();i++) cout << "(" << points[i]->x << "," << points[i]->y << ")" << endl;
			}
			int s1, s2, s;
			if(left!=NULL){
				s1 = left->points.size();
				if(left->p1 != NULL) cout << "lp1(" << left->p1->x << "," << left->p1->y << ") ";
				if(left->p2 != NULL) cout << "lp2(" << left->p2->x << "," << left->p2->y << ")";
				cout << endl;
			} else s1 = 0;
			if(right!=NULL){
				s2 = right->points.size();
				if(right->p1 != NULL) cout << "rp1(" << right->p1->x << "," << right->p1->y << ") ";
				if(right->p2 != NULL) cout << "rp2(" << right->p2->x << "," << right->p2->y << ")";
				cout << endl;
			} else s2 = 0;
			s = (s1>s2) ? s1 : s2;
			//cout << "s:" << s << "," << "s1:" << s1 << "," << "s2:" << s2 << endl;
			for(int i=0;i<s;i++){
				if(s1 > i) cout << "l(" << left->points[i]->x << "," << left->points[i]->y << ")";
				cout << "\t\t";
				if(s2 > i) cout << "r(" << right->points[i]->x << "," << right->points[i]->y << ")";
				cout << endl;
			}
			if(left!=NULL){
				cout << "LEFT:" << endl;
				left->printtree();
			}
			if(right!=NULL){
				cout << "RIGHT:" << endl;
				right->printtree();
			}
		}

		void initialize(){
			int size = points.size();
			if(size<2) return;
			else if(size==2){
				p1 = points[0];
				p2 = points[1];
				points.clear();
				return;
			}
			vector<int> arr;
			if(size>150){
				for(int i=0;i<150;i++){
					arr.push_back(rand() % size);
				}
			} else {
				for(int i=0;i<size;i++){
					arr.push_back(i);
				}
			}
			double max = 0;
			int ptr1, ptr2;
			for(int i=1;i<arr.size();i++){
				for(int j=0;j<i;j++){
					double d = distance(points[arr[i]],points[arr[j]]);
					if(d > max){
						max = d;
						if(arr[i]>arr[j]){
							ptr1 = arr[i];
							ptr2 = arr[j];							
						} else {
							ptr1 = arr[j];
							ptr2 = arr[i];
						}
					}
				}
			}
			//cout << ptr1 << ptr2 << endl;
			p1 = points[ptr1]; p2 = points[ptr2];
			
			left = new ghTree();
			right = new ghTree();

			points.erase(points.begin()+ptr1);
			points.erase(points.begin()+ptr2);

			for(int i=0;i<points.size();i++){
				//cout << i << endl;
				if(distance(p1,points[i]) < distance(p2,points[i])){
					//cout << "to left" << endl;
					left->add_point(points[i]);
				} else {
					//cout << "to right" << endl;
					right->add_point(points[i]);
				}
			}
			points.clear();
			left->initialize();
			right->initialize();
			if(left->p1==NULL && left->points.empty()) left=NULL;
			if(right->p1==NULL && right->points.empty()) right=NULL;

		}

		void add_point(Point *pt){
			points.push_back(pt);
		}

		void range_search(Point *pt, double range, list<Point*>& res){
			if(p1!=NULL){
				if(distance(p1,pt)<=range) res.push_back(p1);
				if(distance(p2,pt)<=range) res.push_back(p2);
				double lb = maximum(0, (distance(pt,p1) - distance(pt,p2))/2);
				if(lb <= range && left!=NULL) left->range_search(pt,range,res);
				lb = maximum(0, (distance(pt,p2) - distance(pt,p1))/2);
				if(lb <= range && right!=NULL) right->range_search(pt,range,res);
			} else {
				for(int i=0;i<points.size();i++){
					if(distance(pt,points[i])<range) res.push_back(points[i]);
				}
			}
		}

		void knn_search(Point *pt, int k, myqueue *queue){
			if(k==0) return;
			if(p1!=NULL){
				
				element *e = new element;
				e->p = p1;
				e->dist = distance(pt,p1);
				if(e->dist > 0){
					if(queue->size() < k) queue->push(e);
					else {
						if(queue->top()->dist > e->dist){
							queue->pop();
							queue->push(e);
						}
					}
				}
				e = new element;
				e->p = p2;
				e->dist = distance(pt,p2);
				if(e->dist > 0){
					if(queue->size() < k) queue->push(e);
					else {
						if(queue->top()->dist > e->dist){
							queue->pop();
							queue->push(e);
						}
					}
				}
				double lb = maximum(0, (distance(pt,p1) - distance(pt,p2))/2);
				if(lb <= queue->top()->dist && left!=NULL) left->knn_search(pt,k,queue);
				lb = maximum(0, (distance(pt,p2) - distance(pt,p1))/2);
				if(lb <= queue->top()->dist && right!=NULL) right->knn_search(pt,k,queue);

			} else {
				for(int i=0;i<points.size();i++){
					element *e = new element;
					e->p = points[i];
					e->dist = distance(pt,points[i]);
					if(e->dist <= 0) continue;
					if(queue->size() < k) queue->push(e);
					else {
						if(queue->top()->dist > e->dist){
							queue->pop();
							queue->push(e);
						}
					}
				}
			}	
		}

};

ghTree *root;

void query(string query_line, int flag){

	//if(flag>0)
	string args[4];
	string arg;
	istringstream line(query_line);
	//cout << query_line  << endl;
	cout << ".";
	int i = 0;
	while(line >> arg){
		args[i] = arg;
		i++;
	}
	
	int query_type = atoi(args[0].c_str());
	
	if(query_type==2){
		double x,y,r;
		x = atof(args[1].c_str());
		y = atof(args[2].c_str());
		r = atof(args[3].c_str());
		list<Point*> res;
		counter = 0;
		start = clock();
		root->range_search(new Point(x,y), r, res);
		end = clock();
		rdistance_counts.push_back(counter);
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		for(list<Point*>::iterator it = res.begin(); it!=res.end();++it){
			cout << "(" << (*it)->x << "," << (*it)->y << "," << (*it)->id << "," << (*it)->obj << ")" << endl;
		}
	} else if(query_type==3){
		double x,y;
		x = atof(args[1].c_str());
		y = atof(args[2].c_str());
		int k = atoi(args[3].c_str());
		myqueue queue;
		list<element*> res;
		start = clock();
		counter = 0;
		root->knn_search(new Point(x,y), k, &queue);
		end = clock();
		kdistance_counts.push_back(counter);
		knn_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*while(!queue.empty()){
			element *e = queue.top();
			res.push_front(e);
			queue.pop();
		}
		for(list<element*>::iterator it = res.begin(); it!=res.end();++it){
			element *e = (*it);
			cout << "(" << e->dist << "," << e->p->x << "," << e->p->y << "," << e->p->id << "," << e->p->obj << ")" << endl;
		}*/

	}
	
	//if(flag>0)
	//cout << "\n" ;
}

double sta_dev(vector<double> v, double mean){
	vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	return sqrt(sq_sum / v.size());
}

int main(int argc, char *argv[]){

	root = new ghTree();
	if(argc > 1){
		dist_func = atoi(argv[1]);
	}
	ifstream mat_file("assgn5_matrix.txt");
	mat_file >> matrix[0][0];
	mat_file >> matrix[0][1];
	mat_file >> matrix[1][0];
	mat_file >> matrix[1][1];
	
	string query_line;
	ifstream file1("assgn5_data.txt");
	//ifstream file1("d.txt");
	if(file1.is_open()){
		while(getline(file1, query_line)){
			istringstream line(query_line);
			double x,y;
			int id;
			string obj;
			line >> x;
			line >> y;
			line >> id;
			line >> obj;
			root->add_point(new Point(x,y,id,obj));
		}
	} else cout << "Error opening file1.." << endl;
	root->initialize();
	//root->printtree();

	//ifstream file("assgn5_querysample.txt");
	ifstream file("q.txt");
	if(file.is_open()){
		while(getline(file, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening file.." <<endl;
	//root->printtree();
	file.close();

	char *name = (char *) malloc (15*sizeof(char));
	sprintf(name,"ghTree_%d.out",dist_func);
	ofstream output(name);
	double sum, mean, sq_sum, st_dev;
	output << "\nRange Search stats:\n";
	output << "min - " << *min_element(range_search_times.begin(), range_search_times.end()) << "\n";
	output << "max - " << *max_element(range_search_times.begin(), range_search_times.end()) << "\n";
	sum = std::accumulate(range_search_times.begin(), range_search_times.end(), 0.0);
	mean = sum / range_search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(range_search_times, mean) << "\n";
	output << "\nRange Search Distance stats:\n";
	output << "min - " << *min_element(rdistance_counts.begin(), rdistance_counts.end()) << "\n";
	output << "max - " << *max_element(rdistance_counts.begin(), rdistance_counts.end()) << "\n";
	sum = std::accumulate(rdistance_counts.begin(), rdistance_counts.end(), 0.0);
	mean = sum / rdistance_counts.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(rdistance_counts, mean) << "\n";
	output << "\nknn Search stats:\n";
	output << "min - " << *min_element(knn_search_times.begin(), knn_search_times.end()) << "\n";
	output << "max - " << *max_element(knn_search_times.begin(), knn_search_times.end()) << "\n";
	sum = std::accumulate(knn_search_times.begin(), knn_search_times.end(), 0.0);
	mean = sum / knn_search_times.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(knn_search_times, mean) << "\n";
	output << "\nknn-search Distance stats:\n";
	output << "min - " << *min_element(kdistance_counts.begin(), kdistance_counts.end()) << "\n";
	output << "max - " << *max_element(kdistance_counts.begin(), kdistance_counts.end()) << "\n";
	sum = std::accumulate(kdistance_counts.begin(), kdistance_counts.end(), 0.0);
	mean = sum / kdistance_counts.size();
	output << "mean - " << mean << "\n";
	output << "standard deviation - " << sta_dev(kdistance_counts, mean) << "\n";
	output.close();


}