#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <list>
#include <math.h>
#include <vector>
#include <functional>
#include <queue>
#include <time.h>
#include <iomanip>
#include <numeric>
#include <algorithm>

using namespace std;

class Point{
	public:
		double x;
		double y;
	Point(double x_cod, double y_cod){
		x = x_cod;
		y = y_cod;
	}
};

clock_t start, end;
vector<double> insert_times;
vector<double> search_times;
vector<double> range_search_times;
vector<double> knn_search_times;
vector<double> window_search_times;

class KdTree;

KdTree *root;

int counter;

struct node{
	KdTree *data;
	double dist;
};

struct MyComparator{
    bool operator() (node *n1, node *n2){
         return (n1->dist) > (n2->dist);
    }
};

typedef priority_queue<node*, vector<node*>, MyComparator> myqueue;

double distance(double x1, double y1, double x2, double y2){
	double dx = (x2-x1);
	double dy = (y2-y1);
	return sqrt(dx*dx + dy*dy);
}

double dist_rect(Point *dl, Point *dr, double x_c, double y_c){
	double dx, dy;
	if(x_c < dl->x) dx = (dl->x - x_c);
	else if(x_c > dr->x) dx = (x_c - dr->x);
	else dx = 0;
	if(y_c < dl->y) dy = (dl->y - y_c);
	else if(y_c > dr->y) dy = (y_c - dr->y);
	else dy = 0;
	return sqrt(dx*dx+dy*dy);
}
				

/**
-----------------------dr
|	q4  |	q3	|
|	    |		|
|---------center--------|
|	q1  |   q2	|
|	    |	  	|
dl-----------------------
**/

class KdTree{
	public:
		double x;
		double y;
		bool iscenter;
		KdTree *q1;
		KdTree *q2;
		char split;
		Point *dl;
		Point *dr;

		KdTree(double x_cod, double y_cod){
			x = x_cod;
			y = y_cod;
			iscenter = false;
			char split = 'x';
			q1 = NULL;
			q2 = NULL;
			dl = NULL;
			dr = NULL;
		}

		void printtree(){
			cout << "(" << x << ", " << y << "," << split << ") -> ";
			if(q1==NULL){
				cout << "NULL, ";
			} else {
				cout << "(" << q1->x << ", " << q1->y << "," << q1->split << "),";
			}
			if(q2==NULL){
                                cout << "NULL.\n";
                        } else {
                                cout << "(" << q2->x << ", " << q2->y << "," << q2->split << ").\n";
                        }
			if(q1!=NULL) if(q1->iscenter) q1->printtree();
			if(q2!=NULL) if(q2->iscenter) q2->printtree();
		}

		void insert(double x_cod, double y_cod){
			//cout << "INserting " << x_cod << "," << y_cod << "..dl - " << dl->x << "," << dl->y << "..dr - " << dr->x << "," << dr->y <<"..\n";
			double x1 = (dl->x+dr->x)/2;
			double y1 = (dl->y+dr->y)/2;
			if((split=='x' && x_cod<=x1) || (split=='y' && y_cod<=y1)){
				if(q1==NULL){
					q1 = new KdTree(x_cod,y_cod);
					q1->split = (split=='x') ? 'y' : 'x';
					//cout << "l1\n";
				} else if(q1->iscenter){
					q1->insert(x_cod,y_cod);
					//cout << "l2\n";
				} else {
					//cout << "l3\n";
					if(q1->x==x_cod && q1->y==y_cod) return;
					KdTree *temp = q1;
					if(temp->split=='y'){
						q1 = new KdTree((dl->x+dr->x)/2,(dl->y+dr->y)/2);
						q1->iscenter = true;
						q1->dl = dl;
						q1->dr = new Point(q1->x,dr->y);
						q1->split = temp->split;
						q1->insert(temp->x, temp->y);
						free(temp);
						q1->insert(x_cod, y_cod);
					} else {
						q1 = new KdTree((dl->x+dr->x)/2,(dl->y+dr->y)/2);
						q1->iscenter = true;
						q1->dl = dl;
						q1->split = temp->split;
						q1->dr = new Point(dr->x,q1->y);
						q1->insert(temp->x, temp->y);
						free(temp);
						q1->insert(x_cod, y_cod);
					}
				}
			} else if((split=='x' && x_cod>x1) || (split=='y' && y_cod>y1)){
        			if(q2==NULL){
					//cout << "l4\n";
                        		q2 = new KdTree(x_cod,y_cod);
					q2->split = (split=='x') ? 'y' : 'x';
                    		} else if(q2->iscenter){
					//cout << "l5\n";
                        		q2->insert(x_cod,y_cod);
                    		} else {
					//cout << "l6\n";
                        		if(q2->x==x_cod && q2->y==y_cod) return;
                        		KdTree *temp = q2;
                        		if(temp->split=='y'){
						q2 = new KdTree((dl->x+dr->x)/2,(dl->y+dr->y)/2);
						q2->iscenter = true;
						q2->dl = new Point(q2->x,dl->y);
						q2->dr = dr;
						q2->split = temp->split;
						q2->insert(temp->x, temp->y);
						free(temp);
						q2->insert(x_cod, y_cod);
					} else {
						q2 = new KdTree((dl->x+dr->x)/2,(dl->y+dr->y)/2);
						q2->iscenter = true;
						q2->dl = new Point(dl->x,q2->y);
						q2->dr = dr;
						q2->split = temp->split;
						q2->insert(temp->x, temp->y);
						free(temp);
						q2->insert(x_cod, y_cod);
					}
            			}
			}			
		}	//End of insert.


		void search(double x_cod, double y_cod, int *res){
			double x1 = (dl->x+dr->x)/2;
			double y1 = (dl->y+dr->y)/2;
			if((split=='x' && x_cod<=x1) || (split=='y' && y_cod<=y1)){
				if(q1==NULL){
					//Ignore
				} else if(q1->iscenter){
					q1->search(x_cod,y_cod,res);
				} else {
					if(q1->x==x_cod && q1->y==y_cod) *res = 1;
				}
			} else if((split=='x' && x_cod>x1) || (split=='y' && y_cod>y1)){
				if(q2==NULL){
					//Ignore
                                } else if(q2->iscenter){
                                        q2->search(x_cod,y_cod,res);
                                } else {
                                        if(q2->x==x_cod && q2->y==y_cod) *res = 1;
                                }
			}
		}				//End of point search
		
		
		void range_search(double x_c, double y_c, double r, list<KdTree*> *res){
			if(iscenter){
				double dist = dist_rect(dl, dr, x_c, y_c);
				if(dist>r){
					//Dont pass to children.
				} else {
					if(q1!=NULL) q1->range_search(x_c,y_c,r,res);
                                        if(q2!=NULL) q2->range_search(x_c,y_c,r,res);
                    }
			} else {
				if(distance(x_c,y_c,x,y)<=r) res->push_back(this);
			}
		}				//End of range search

		void knn_search(double x_c, double y_c, double k, list<KdTree*> *res, myqueue *queue){
			if(iscenter){
				if(q1!=NULL){
					node *n = new node;
					n->data = q1;
					if(q1->iscenter) n->dist = dist_rect(q1->dl, q1->dr, x_c, y_c);
					else n->dist = distance(q1->x, q1->y, x_c, y_c);
					queue->push(n);
				}
				if(q2!=NULL){
					node *n = new node;
					n->data = q2;
					if(q2->iscenter) n->dist = dist_rect(q2->dl, q2->dr, x_c, y_c);
					else n->dist = distance(q2->x, q2->y, x_c, y_c);
					queue->push(n);
				}
				if(!queue->empty()){
					node *n = queue->top();
					KdTree *temp = n->data;
					queue->pop();
					temp->knn_search(x_c, y_c, k, res, queue);
				}
			} else {
				if(counter>=k) return;
				res->push_back(this);
				counter++;
				if(!queue->empty()){
					node *n = queue->top();
					KdTree *temp = n->data;
					queue->pop();
					temp->knn_search(x_c, y_c, k, res, queue);
				}						
			}
		}

		void window_search(double x1, double y1, double x2, double y2, list<KdTree*> *res){
			if(iscenter){		//Geographical center.
				if(dl->x>x2 || dr->x<x1 || dl->y>y2 || dr->y<y1){
					//Dont pass to child.
				} else {
					if(q1!=NULL) q1->window_search(x1,y1,x2,y2,res);
					if(q2!=NULL) q2->window_search(x1,y1,x2,y2,res);
				}
			} else {		//leaf node.
				if((x>=x1 && x<=x2) && (y>=y1 && y<=y2)){
					res->push_back(this);
				}
			}
		}				// End of window search
		
};

void query(string query_line, int flag){
	double args[5];
	int i=0;
	string arg;
	istringstream line(query_line);
	while(line >> arg){
		args[i] = atof(arg.c_str());
		i++;
	}
	if(flag>0) cout << query_line << endl;
	if(args[0]==0){
		start = clock();
		root->insert(args[1],args[2]);
		end = clock();
		if(flag>0) insert_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		if(flag>0) cout << "Inserted: (" << args[1] << "," << args[2] << ")" << endl;
	} else if(args[0]==1){
		int res=0;
		start = clock();
		root->search(args[1],args[2], &res);
		end = clock();
		search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		if(res==0) cout << "Not found: (" << args[1] << "," << args[2] << ")" << endl;
		else cout << "(" << args[1] << "," << args[2] << ")" << endl;
	} else if(args[0]==2){
		list<KdTree*> res;
		start = clock();
		root->range_search(args[1],args[2],args[3],&res);
		end = clock();
		range_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		for(list<KdTree*>::iterator it = res.begin(); it!=res.end();++it){
                        cout << "(" << (*it)->x << "," << (*it)->y << ")\n";
                }
	} else if(args[0]==3){
		/*head = NULL;
		tail = NULL;*/
		myqueue queue;
		counter = 0;
		list<KdTree*> res;
		start = clock();
		root->knn_search(args[1],args[2],args[3],&res, &queue);
		end = clock();
		knn_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		for(list<KdTree*>::iterator it = res.begin(); it!=res.end();++it){
                        cout << "(" << (*it)->x << "," << (*it)->y << ")\n";
                }
	} else if(args[0]==4){
		if(args[1]>args[3]){
			double temp = args[1];
			args[1] = args[3];
			args[3] = temp;
		}
		if(args[2]>args[4]){
			double temp = args[2];
			args[2] = args[4];
			args[4] = temp;
		}
		list<KdTree*> res;
		start = clock();
		root->window_search(args[1],args[2],args[3],args[4],&res);
		end = clock();
		window_search_times.push_back(((double) (end - start)) * 1000 / CLOCKS_PER_SEC);
		for(list<KdTree*>::iterator it = res.begin(); it!=res.end();++it){
			cout << "(" << (*it)->x << "," << (*it)->y << ")\n";
		}
	}
	if(flag>0) cout << "\n" ;
}

double sta_dev(vector<double> v, double mean){
	vector<double> diff(v.size());
	std::transform(v.begin(), v.end(), diff.begin(),
               std::bind2nd(std::minus<double>(), mean));
	double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	return sqrt(sq_sum / v.size());
}

int main(){
	root = new KdTree(0.5, 0.5);
	root->iscenter = true;
	root->dl = new Point(0,0);
	root->dr = new Point(1,1);
	root->split = 'x';
	string query_line;
	ifstream file1("assgn2testdata_5.txt");
	if(file1.is_open()){
		while(getline(file1, query_line)){
			query("0	" + query_line, 0);
		}
	} else cout << "Error opening file1.." << endl;
	ifstream file("assgn2testquery_5.txt");
//	ifstream file("q.txt");
	if(file.is_open()){
		while(getline(file, query_line)){
			query(query_line, 1);
		}
	} else cout << "Error opening file.." <<endl;
	//root->printtree();
	file.close();
	ofstream output;
	double sum, mean, sq_sum, st_dev;
	output.open("kdtree.out");
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
	
	return 0;
}
