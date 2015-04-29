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
int counter;

class Point{
	public:
		double x;
		double y;		
		int id;
		double dist;
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

struct MyComparator1{
    bool operator() (Point *n1, Point *n2){
         return (n1->dist) > (n2->dist);
    }
};

typedef priority_queue<element*, vector<element*>, MyComparator> myqueue;
typedef priority_queue<Point*, vector<Point*>, MyComparator1> sortedqueue;

class ghTree;

double sta_dev(vector<double> v, double mean){
	vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	return sqrt(sq_sum / v.size());
}

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

class vpTree{

	public:
		Point *p;
		double dmed;
		vector<Point*> points;
		vpTree *left;
		vpTree *right;

		vpTree(){
			left = NULL;
			right = NULL;
			p = NULL;
		}

		void printtree(){
			if(p!=NULL) cout << "\t(" << p->x << "," << p->y << ")" << endl;
			else{
				for(int i=0;i<points.size();i++) cout << "(" << points[i]->x << "," << points[i]->y << ")" << endl;
			}
			int s1, s2, s;
			if(left!=NULL){
				s1 = left->points.size();
				if(left->p != NULL) cout << "lp1(" << left->p->x << "," << left->p->y << ") ";
				cout << endl;
			} else s1 = 0;
			if(right!=NULL){
				s2 = right->points.size();
				if(right->p != NULL) cout << "rp1(" << right->p->x << "," << right->p->y << ") ";
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
			if(size < 1) return;
			if(size==1){
				p = points[0];
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
			double max = 0, sum = 0, mean, st_dev, var;
			int ptr;
			vector<double> distances;
			for(int i=1;i<arr.size();i++){
				for(int j=0;j<arr.size();j++){
					if(i==j){
						distances.push_back(0);
						continue;
					}
					double d = distance(points[arr[i]],points[arr[j]]);
					distances.push_back(d);
					sum += d;
				}
				mean = sum/distances.size();
				st_dev = sta_dev(distances, mean);
				var = st_dev*st_dev;
				if(max < var){
					max = var;
					ptr = arr[i];
				}
				distances.clear();
			}
			p = points[ptr];
			left = new vpTree();
			right = new vpTree();

			points.erase(points.begin()+ptr);
			sortedqueue *queue = new sortedqueue;
			for(int i=0;i<points.size();i++){
				points[i]->dist = distance(points[i],p);
				queue->push(points[i]);
			}
			for(int i=0;i<points.size()/2;i++) queue->pop();
			dmed = distance(p,queue->top());
			
			for(int i=0;i<points.size();i++){
				//cout << i << endl;
				if(points[i]->dist <= dmed){
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
			if(left->p==NULL) left=NULL;
			if(right->p==NULL) right=NULL;
		}

		void add_point(Point *pt){
			points.push_back(pt);
		}

		void range_search(Point *pt, double range, list<Point*>& res){
			if(p!=NULL){
				if(distance(p,pt)<=range) res.push_back(p);
				double lb = maximum(0, (distance(pt,p) - dmed)/2);
				if(lb <= range && left!=NULL) left->range_search(pt,range,res);
				lb = maximum(0, (dmed - distance(pt,p))/2);
				if(lb <= range && right!=NULL) right->range_search(pt,range,res);
			}
		}

		void knn_search(Point *pt, int k, myqueue *queue){
			if(k==0) return;
			if(p!=NULL){
				
				element *e = new element;
				e->p = p;
				e->dist = distance(pt,p);
				if(e->dist > 0){
					if(queue->size() < k) queue->push(e);
					else {
						if(queue->top()->dist > e->dist){
							queue->pop();
							queue->push(e);
						}
					}
				}
				double lb = maximum(0, (distance(pt,p) - dmed)/2);
				if(lb < queue->top()->dist && left!=NULL) left->knn_search(pt,k,queue);
				lb = maximum(0, (dmed - distance(pt,p))/2);
				if(lb < queue->top()->dist && right!=NULL) right->knn_search(pt,k,queue);
			}
		}

};

vpTree *root;

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
		start = clock();
		root->range_search(new Point(x,y), r, res);
		end = clock();
		rdistance_counts.push_back(counter);
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		/*for(list<Point*>::iterator it = res.begin(); it!=res.end();++it){
			cout << "(" << (*it)->x << "," << (*it)->y << "," << (*it)->id << "," << (*it)->obj << ")" << endl;
		}*/
	} else if(query_type==3){
		double x,y;
		x = atof(args[1].c_str());
		y = atof(args[2].c_str());
		int k = atoi(args[3].c_str());
		myqueue queue;
		list<element*> res;
		start = clock();
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

int main(int argc, char *argv[]){

	root = new vpTree();
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
	ifstream file("assgn5_querysample.txt");
	//ifstream file("q.txt");
	if(file.is_open()){
		while(getline(file, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening file.." <<endl;
	file.close();

	char *name = (char *) malloc (15*sizeof(char));
	sprintf(name,"vpTree_%d.out",dist_func);
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