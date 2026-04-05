#ifndef _GRAPH_H
#define _GRAPH_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <utility>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdint>
#include <sys/time.h>
#include <bitset>
#include "Util.h"
using namespace std;

typedef map<int,vector<int> > HashMap;
const int Kp = 64; // Size of LandBit bitset
const int Kt = 64; // Size of CR bitset

class ReachIndex{
public:
	uint16_t RevTopoOrder;
	uint16_t TopoOrder;
	bitset<Kp> Pb_in, Pb_out;
};

class IndexTopo{
public:
	uint16_t RevTopoOrder;
	uint16_t TopoOrder;
};

class IndexPLL{
public:
	bitset<Kp> Pb_in, Pb_out;
};

class Graph{
	public:
		int HUGENODENUM;
		int hugenode;
		int vsize;
		int esize;
		string name;
		// GRA graph;
		vector<int> Inptr;
		vector<int> InEdges;
		vector<int> NewInEdges;
		vector<int> degreeIn;
		vector<int> Outptr;
		vector<int> OutEdges;
		vector<int> NewOutEdges;
		vector<int> degreeOut;
		// Graph Index
		vector<bitset<Kt>> CR;
		vector<ReachIndex> reachindex;
		vector<IndexTopo> topoindex;
		vector<IndexPLL> landbit;
		vector<uint16_t>RevTopoOrder;
		vector<uint16_t>TopoOrder;
		vector<bitset<Kp>> Pb_in, Pb_out;
		
		// vector<int> stime, etime;
		Graph();
		Graph(int);
		Graph(istream&);
		Graph(const Graph& g);
		~Graph();
		void readGraph(istream&);
		void genRandomGraph(int n, double c, char* filename);
		void clear();
		void strTrimRight(string& str);
		void scalability(int add_cnt);
		uint64_t outdegree(int vertex);
		uint64_t indegree(int vertex);

		string getFilename();
		void setFilename(string name);
		static vector<string> split(const string &s, char delim);
		static vector<string>& split(const string &s, char delim, vector<string> &elems);

		bool bfs_check(int u, int v) {
			vector<int> vis(vsize, 0);
			queue<int> q;
			q.push(u);
			vis[u] = 1;
			while (!q.empty()) {
				int t = q.front();
				q.pop();
				for (int i = Outptr[t]; i < Outptr[t + 1]; i++) {
					if (OutEdges[i] == v) return true;
					if (vis[OutEdges[i]]) continue;
					q.push(OutEdges[i]);
					vis[OutEdges[i]] = 1;
				}
			}
			return false;
		}
private:
	void ReadIndex1(const string&);
	void ReadIndex0(const string&);
public:
		void delRedounEdge(vector<bool> isPLL);
		bool OPT_DFS1(const int u, const int v);
		bool OPT_DFS0(const int u, const int v);
		bool CompressionDFS(const int u, const int v);
		bool ProLabelQueryBFS(const int u, const int v);
		void QueryTest1(const string& queryfilename, const string& queryIndex);
		void QueryTest0(const string& queryfilename, const string& queryIndex);
		void readQueryFile(const string& queryfilename);
		void runQuery1(vector<int> &src, vector<int> &dest, vector<int> &reachflag);
		void runQuery0(vector<int> &src, vector<int> &dest, vector<int> &reachflag);
	};

class Topo {
public:
	vector<uint16_t> TopoOrder;
	vector<uint16_t> RevTopoOrder;
    Topo(Graph* graph);
};

class LandBitIndex {
public:
	vector<bitset<Kp>> Pb_in;
    vector<bitset<Kp>> Pb_out;
    vector<bool> IsPLL;
    LandBitIndex(Graph* graph);
};

class CRIndex {
public:
    vector<bitset<Kt>> reach_bit;
    CRIndex(Graph* graph, LandBitIndex* pll, Topo* topo);
};
#endif

