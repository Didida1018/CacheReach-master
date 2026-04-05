#ifndef GRAPH_CPP_
#define GRAPH_CPP_

#include "Graph.h"
#include <numeric>
#include <random>
#include <atomic>
#include <omp.h>

int* flag;
int querycnt;
long long dfscount = 0;

Graph::Graph() {
    vsize=0;
}

Graph::Graph(int size) {
    vsize = size;
}

Graph::Graph(istream& in) {
    readGraph(in);
}

Graph::~Graph() {}

void Graph::clear() {
    vsize = 0;
    Inptr.clear();
    InEdges.clear();
    Outptr.clear();
    OutEdges.clear();
}

void Graph::strTrimRight(string& str) {
    string whitespaces(" \t\r\n");
    int index = str.find_last_not_of(whitespaces);
    if (index != string::npos) 
        str.erase(index+1);
    else
        str.clear();
}


void Graph::readGraph(istream& in) {
    string buf;
    getline(in, buf);
    strTrimRight(buf);
    if (buf != "graph_for_greach") {
        cout << buf << " BAD FILE FORMAT!" << endl;
        exit(0);
    }

    getline(in, buf);
    int n;
    istringstream(buf) >> n;
    vsize = n;

    int outEdgeCount = 0;
    int begin, end;
    int sid, tid;
    vector<string> neighbors;
    vector<vector<int>> InGraph;
    InGraph.resize(n);

    while (getline(in, buf)) {
        begin = buf.find(":");
        end = buf.find_last_of("#");
        sid = atoi((buf.substr(0, begin)).c_str());
        buf = buf.substr(begin + 2, end - begin - 2);
        split(buf, ' ', neighbors);
        Outptr.push_back(outEdgeCount);
        outEdgeCount += neighbors.size();
        for (int i = 0; i < neighbors.size(); i++) {
            tid = atoi(neighbors[i].c_str());
            OutEdges.push_back(tid);
            InGraph[tid].push_back(sid);
        }
        neighbors.clear();
    }
    Outptr.push_back(outEdgeCount);

    int inEdgeCount = 0;
    for (int i = 0; i < n; i++) {
        for (const int w : InGraph[i]) InEdges.push_back(w);
        Inptr.push_back(inEdgeCount);
        inEdgeCount += InGraph[i].size();
    }
    Inptr.push_back(inEdgeCount);

    esize = outEdgeCount; // Ensure esize is set to the total number of edges
}

void Graph::readQueryFile(const string& queryfilename) {
    ifstream queryfile(queryfilename.c_str());
    string line;
    vector<int> src;
    vector<int> dest;
    vector<int> reachflag;
    vector<string> tmps;
    int i = 0;
    while (getline(queryfile, line), line.size()) {
        split(line, ' ', tmps);
        src.push_back(atoi(tmps[0].c_str()));
        dest.push_back(atoi(tmps[1].c_str()));
        reachflag.push_back(atoi(tmps[2].c_str()));
        i++;
        tmps.clear();
    }
    queryfile.close();
}

vector<string>& Graph::split(const string &s, char delim, vector<string> &elems) {
	int begin, end;

	begin=0;
	end=s.find(delim);
	while(end!=string::npos){
		elems.push_back(s.substr(begin, end-begin));
		begin=end+1;
		end=s.find(delim, begin);
	}
	if(begin!=s.size()){
		elems.push_back(s.substr(begin));
	}

	return elems;
}

vector<string> Graph::split(const string &s, char delim) {
	vector<string> elems;
	return split(s, delim, elems);
}

uint64_t Graph::outdegree(int vertex) {
    return Outptr[vertex + 1] - Outptr[vertex];
}

uint64_t Graph::indegree(int vertex) {
    return Inptr[vertex + 1] - Inptr[vertex];
}

string Graph::getFilename(){
	return name;
}
void Graph::setFilename(string name){
	this->name.swap(name);
}

void Graph::ReadIndex1(const string& queryIndex){
	ifstream Graphbit(queryIndex);

	if(Graphbit.is_open()==false){
		cout << "Cannot open " << queryIndex << endl;
		quit();
	}
	reachindex.resize(vsize);
	CR.resize(vsize);
	long long count = 0;
	for (int i = 0; i < vsize; i++) {
		Graphbit.read((char *)&CR[i], sizeof(CR[i]));
		Graphbit.read((char *)&reachindex[i].Pb_in, sizeof(reachindex[i].Pb_in));
		Graphbit.read((char *)&reachindex[i].Pb_out, sizeof(reachindex[i].Pb_out));
		Graphbit.read((char *)&reachindex[i].TopoOrder, sizeof(reachindex[i].TopoOrder));
		Graphbit.read((char *)&reachindex[i].RevTopoOrder, sizeof(reachindex[i].RevTopoOrder));
	}
    double vs =  sizeof(reachindex[0]);
	Graphbit.close();
}

void Graph::ReadIndex0(const string& queryIndex){
	ifstream Graphbit(queryIndex);
	if(Graphbit.is_open()==false){
		cout << "Cannot open " << queryIndex << endl;
		quit();
	}
	CR.resize(vsize);
    landbit.resize(vsize);
    TopoOrder.resize(vsize);
    RevTopoOrder.resize(vsize);
	long long count = 0;
	for (int i = 0; i < vsize; i++) {
		Graphbit.read((char *)&CR[i], sizeof(CR[i]));
        Graphbit.read((char *)&landbit[i].Pb_in, sizeof(landbit[i].Pb_in));
        Graphbit.read((char *)&landbit[i].Pb_out, sizeof(landbit[i].Pb_out));
        Graphbit.read((char *)&TopoOrder[i], sizeof(TopoOrder[i]));
        Graphbit.read((char *)&RevTopoOrder[i], sizeof(RevTopoOrder[i]));
	}
	Graphbit.close();
}


int TopoErr = 0, CRErr = 0;
int NoneCnt = 0;
int CRCnt = 0, TopoCnt=0;
int Cnt = 0;
int PLLCnt = 0;
int reach=0,nonreach=0;

void Graph::runQuery1(vector<int> &src, vector<int> &dest, vector<int> &reachflag) {  
    bool ProLflag;
    for(vector<int>::iterator uit=src.begin(), vit=dest.begin(); uit!=src.end(); uit++, vit++){
        if ((CR[*uit] & CR[*vit]).any()
            || reachindex[*uit].TopoOrder>=reachindex[*vit].TopoOrder
			|| reachindex[*uit].RevTopoOrder<=reachindex[*vit].RevTopoOrder
            || (reachindex[*vit].Pb_in & reachindex[*uit].Pb_in) != reachindex[*uit].Pb_in
			|| (reachindex[*uit].Pb_out & reachindex[*vit].Pb_out) != reachindex[*vit].Pb_out)
            ProLflag = false;
        else if((reachindex[*uit].Pb_out & reachindex[*vit].Pb_in).any())
            ProLflag = true;
        else{
			querycnt++;
			OPT_DFS1(*uit, *vit);
		}
	}
}


void Graph::runQuery0(vector<int> &src, vector<int> &dest, vector<int> &reachflag) {
    bool ProLflag;
    for(vector<int>::iterator uit=src.begin(), vit=dest.begin(); uit!=src.end(); uit++, vit++){
        if ((CR[*uit] & CR[*vit]).any()
            || TopoOrder[*uit]>=TopoOrder[*vit] 
            || RevTopoOrder[*uit]<=RevTopoOrder[*vit]
            || (landbit[*vit].Pb_in & landbit[*uit].Pb_in) != landbit[*uit].Pb_in
			|| (landbit[*uit].Pb_out & landbit[*vit].Pb_out) != landbit[*vit].Pb_out
		) ProLflag = false;
        else if ((landbit[*uit].Pb_out & landbit[*vit].Pb_in).any()) ProLflag = true;
		else{
			querycnt++;
			OPT_DFS0(*uit, *vit);
		}
	}
}

void Graph::QueryTest1(const string& queryfilename, const string& queryIndex){
	int u, v;
	bool ProLflag;
	clock_t start, end;
	struct timeval querystart, queryend;
    // Reading Index
	start=clock();
	vector<bool> isPLL(vsize, false);
	ReadIndex1(queryIndex);
	end=clock();

    // Reading queryfile
	ifstream queryfile(queryfilename.c_str());
	string line;
	vector<int> src, dest, reachflag;
	vector<string> tmps;
	int i=0;
    cout<<"loading..."<<endl;;
	start=clock();
    // while(getline(queryfile, line), line.size()){
    while(getline(queryfile, line)&&!line.empty()){
		split(line, ' ', tmps);
		src.push_back(atoi(tmps[0].c_str()));
		dest.push_back(atoi(tmps[1].c_str()));
		reachflag.push_back(atoi(tmps[2].c_str()));
        i++;
		tmps.clear();
	}
	end=clock();
	queryfile.close();
    cout<<"Query size:"<<src.size()<<endl;
    // query
    querycnt = 0;
    flag=new int[vsize];
	memset(flag, 0, sizeof(int)*vsize);
    gettimeofday(&querystart, NULL);
    runQuery1(src, dest, reachflag);
    gettimeofday(&queryend, NULL);
    double querytime = (queryend.tv_sec - querystart.tv_sec) * 1000.0 + (queryend.tv_usec - querystart.tv_usec) * 1.0 / 1000.0;
    cout << "querytime: " << querytime << endl;
    // verify
    int dfsCount = 0; i = 0;
    memset(flag, 0, sizeof(int)*vsize);
    querycnt = 0;
    for(vector<int>::iterator uit=src.begin(), vit=dest.begin(); uit!=src.end(); uit++, vit++, i++){
        if ((CR[*uit] & CR[*vit]).any()
            || reachindex[*uit].TopoOrder>=reachindex[*vit].TopoOrder
            || reachindex[*uit].RevTopoOrder<=reachindex[*vit].RevTopoOrder
            || (reachindex[*vit].Pb_in & reachindex[*uit].Pb_in) != reachindex[*uit].Pb_in
            || (reachindex[*uit].Pb_out & reachindex[*vit].Pb_out) != reachindex[*vit].Pb_out)
            ProLflag = false;
        else if((reachindex[*uit].Pb_out & reachindex[*vit].Pb_in).any())
            ProLflag = true;
        else{
			querycnt++;
            ProLflag = OPT_DFS1(*uit, *vit);
        }
        if((bool)reachflag[i]!=ProLflag){
            cout << "i: " << i << '\t' << *uit << ' ' << *vit << ' ' << reachflag[i] << endl;
        }
    }
	delete[] flag;
}





void Graph::QueryTest0(const string& queryfilename, const string& queryIndex){
	int u, v;
	bool ProLflag;
	clock_t start, end;
	struct timeval querystart, queryend;

    // Read Index
    start=clock();
    ReadIndex0(queryIndex);
	end=clock();

    // Read queryfile
    ifstream queryfile(queryfilename.c_str());
	string line;
	vector<int> src, dest, reachflag;
	vector<string> tmps;
	int i=0;
    cout<<"loading..."<<endl;;
	start=clock();
	// while(getline(queryfile, line), line.size()){
    while(getline(queryfile, line)&&!line.empty()){
		split(line, ' ', tmps);
		src.push_back(atoi(tmps[0].c_str()));
		dest.push_back(atoi(tmps[1].c_str()));
		reachflag.push_back(atoi(tmps[2].c_str()));
		i++;
		tmps.clear();
	}
	end=clock();
	queryfile.close();
    cout<<"Query size:"<<src.size()<<endl;
    // Query
    querycnt = 0;
	flag=new int[vsize];
	memset(flag, 0, sizeof(int)*vsize);
    gettimeofday(&querystart, NULL);
    runQuery0(src, dest, reachflag);
    gettimeofday(&queryend, NULL);
	double querytime = (queryend.tv_sec - querystart.tv_sec) * 1000.0 + (queryend.tv_usec - querystart.tv_usec) * 1.0 / 1000.0;
    cout << "querytime: " << querytime << endl;
    // verify
    i = 0, querycnt = 0;
    memset(flag, 0, sizeof(int)*vsize);
    for(vector<int>::iterator uit=src.begin(), vit=dest.begin(); uit!=src.end(); uit++, vit++, i++){
        if (
            (CR[*uit] & CR[*vit]).any()
            || TopoOrder[*uit]>=TopoOrder[*vit] 
			|| RevTopoOrder[*uit]<=RevTopoOrder[*vit]
            || (landbit[*vit].Pb_in & landbit[*uit].Pb_in) != landbit[*uit].Pb_in
			|| (landbit[*uit].Pb_out & landbit[*vit].Pb_out) != landbit[*vit].Pb_out
		) ProLflag = false;
        else if ((landbit[*uit].Pb_out & landbit[*vit].Pb_in).any()) ProLflag = true;
		else{
			querycnt++;
			ProLflag = OPT_DFS0(*uit, *vit);
		}
        if((bool)reachflag[i]!=ProLflag){
            cout << "i: " << i << '\t' << *uit << ' ' << *vit << ' ' << reachflag[i] << endl;
        }
    }
	delete[] flag;
}

bool Graph::OPT_DFS1(const int u, const int v){
        flag[u]=querycnt;
		for (vector<int>::iterator it = OutEdges.begin() + Outptr[u]; it != OutEdges.begin() + Outptr[u + 1]; it++) {
            if(*it==v) return true;
			if ((CR[*it] & CR[v]).none()
                && reachindex[*it].TopoOrder < reachindex[v].TopoOrder
                && reachindex[*it].RevTopoOrder > reachindex[v].RevTopoOrder
                && (reachindex[*it].Pb_in & reachindex[*it].Pb_out).none() 
                && (reachindex[*it].Pb_out & reachindex[v].Pb_out) == reachindex[v].Pb_out 
                && (reachindex[*it].Pb_in & reachindex[v].Pb_in) == reachindex[*it].Pb_in 
				&& flag[*it]!=querycnt){
                    if(OPT_DFS1(*it, v))
                        return true;
                }
        }
        return false;
}

bool Graph::OPT_DFS0(const int u, const int v){
        flag[u]=querycnt;
        for (vector<int>::iterator it = OutEdges.begin() + Outptr[u]; it != OutEdges.begin() + Outptr[u + 1]; it++) {
			if(*it==v) return true;
			if ((CR[*it] & CR[v]).none()
                && TopoOrder[*it] < TopoOrder[v] 
                && RevTopoOrder[*it] > RevTopoOrder[v] 
                && (landbit[*it].Pb_out & landbit[*it].Pb_in).none()
                && (landbit[*it].Pb_in & landbit[v].Pb_in) == landbit[*it].Pb_in
                && (landbit[*it].Pb_out & landbit[v].Pb_out) == landbit[v].Pb_out
                && flag[*it]!=querycnt ) {
                    if(OPT_DFS0(*it, v))
                        return true;
			    }
        }
        return false;
}

void BFS_MIS(const std::vector<int>& Edges, const std::vector<int>& ptr, std::vector<int>& visit, int vertex, int k) {
    std::vector<int> current_level, next_level;
    current_level.push_back(vertex);
    visit[vertex] = k;
    while (!current_level.empty()) {
        next_level.clear();
        for (int node : current_level) {
            for (int i = ptr[node]; i < ptr[node + 1]; i++) {
                int neighbor = Edges[i];
                if (visit[neighbor] != k) {
                    visit[neighbor] = k;
                    next_level.push_back(neighbor);
                }
            }
        }
        std::swap(current_level, next_level);
    }
}

void BFS_LandBit(const std::vector<int>& Edges, const std::vector<int>& ptr, std::vector<int>& visit, int vertex, int k, vector<bitset<Kp>> &Pb) {
    queue<int> Queue;
    Queue.push(vertex);
    visit[vertex] = k;
    while (!Queue.empty()) {
        int f = Queue.front();
        Queue.pop();
        Pb[f][k] = 1;
        for (int i = ptr[f]; i < ptr[f + 1]; i++) {
            if(Pb[Edges[i]][k]==0){
                Pb[Edges[i]][k]=1;
                Queue.push(Edges[i]);
            }
        }
    }
}

// Landbit construction
LandBitIndex::LandBitIndex(Graph* graph) {
    Pb_in.resize(graph->vsize);
    Pb_out.resize(graph->vsize);
    IsPLL.resize(graph->vsize);
	vector<int> visit(graph->vsize, -1);
    vector<int> vertex_order(graph->vsize);
	iota(vertex_order.begin(), vertex_order.end(), 0);
    sort(vertex_order.begin(), vertex_order.end(), [&](int vertex_first, int vertex_second){
        return graph->outdegree(vertex_first) * graph->indegree(vertex_first)
            > graph->outdegree(vertex_second) * graph->indegree(vertex_second);
    });
	for(int k = 0; k < Kp; k++) {
		IsPLL[vertex_order[k]] = 1;
		BFS_LandBit(graph->OutEdges, graph->Outptr, visit, vertex_order[k], k, Pb_in);
		BFS_LandBit(graph->InEdges, graph->Inptr, visit, vertex_order[k], k, Pb_out);
	}
}

// CR-vector construction
CRIndex::CRIndex(Graph* graph, LandBitIndex* pll, Topo* topo) {
    int vsize = graph->vsize;
    vector<int> cnt(vsize,0);
    vector<int> Verts(vsize);
    iota(Verts.begin(), Verts.end(), 0);
    vector<int> visit(vsize,-1);
    reach_bit.resize(graph->vsize);
    for(int k=0;k<Kt;k++){
        vector<vector<int>> buckets(k+1);
        if(k==0){
            sort(Verts.begin(), Verts.end(), [&](int vertex_first, int vertex_second){
                return graph->outdegree(vertex_first) * graph->indegree(vertex_first)
                    < graph->outdegree(vertex_second) * graph->indegree(vertex_second);
            });
            for(int v: Verts){
                if(pll->IsPLL[v] == 0)
                    buckets[0].push_back(v);
            }
        }
        else if(k==1){
            sort(Verts.begin(), Verts.end(), [&](int vertex_first, int vertex_second){
                return graph->outdegree(vertex_first) * graph->indegree(vertex_first)
                    > graph->outdegree(vertex_second) * graph->indegree(vertex_second);
            });
            for(int v: Verts){
                if(pll->IsPLL[v] == 0)
                    buckets[0].push_back(v);
            }
        }
        else{
            for(int v=0;v<vsize;v++){
                if(pll->IsPLL[v] == 0)
                    buckets[cnt[v]].push_back(v);
            }
        }
        for(int i=0;i<=k;i++){
            for(auto v: buckets[i]){
                if(visit[v] != k){
                    BFS_MIS(graph->OutEdges, graph->Outptr, visit, v, k);
                    BFS_MIS(graph->InEdges, graph->Inptr, visit, v, k);
                    reach_bit[v][k] = 1;
                    cnt[v]++;
                }
            }
        }
    }
}

Topo::Topo(Graph* graph) {
    int vsize = graph->vsize;
    TopoOrder.resize(vsize, -1);
    RevTopoOrder.resize(vsize, -1);
    vector<int> in_degree(vsize);
    vector<int> out_degree(vsize);
    for (int i = 0; i < vsize; i++) {
        in_degree[i] = graph->Inptr[i + 1] - graph->Inptr[i];  
        out_degree[i] = graph->Outptr[i + 1] - graph->Outptr[i]; 
    }
    std::vector<int> queue;
    queue.reserve(vsize); 

    int front = 0;
    for (int i = 0; i < vsize; i++) {
        if (in_degree[i] == 0) {
            queue.push_back(i);
            TopoOrder[i] = 0;
        }
    }
    while (front < queue.size()) {
        int current = queue[front++];
        int current_order = TopoOrder[current];
        int start = graph->Outptr[current];
        int end = graph->Outptr[current + 1];
        for (int i = start; i < end; i++) {
            int neighbor = graph->OutEdges[i];
            if (--in_degree[neighbor] == 0) {
                TopoOrder[neighbor] = current_order + 1;
                queue.push_back(neighbor);
            }
        }
    }
    queue.clear();
    // Reverse
    front = 0;
    for (int i = 0; i < vsize; i++) {
        if (out_degree[i] == 0) {
            queue.push_back(i);
            RevTopoOrder[i] = 0;
        }
    }
    while (front < queue.size()) {
        int current = queue[front++];
        int current_rev_order = RevTopoOrder[current];
        int start = graph->Inptr[current];
        int end = graph->Inptr[current + 1];
        for (int i = start; i < end; i++) {
            int neighbor = graph->InEdges[i];
            if (--out_degree[neighbor] == 0) {
                RevTopoOrder[neighbor] = current_rev_order + 1;
                queue.push_back(neighbor);
            }
        }
    }
}
#endif
