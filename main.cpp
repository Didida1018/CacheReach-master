#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <set>
#include <functional>
#include <climits>
#include <ctime>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "Graph.h"
#include "Util.h"

using namespace std;

void usage(){
    cout << "Usage:\n"
        "CacheReach [--help] <filename> [-i <indexfile>] [-l <layout>] [-q <queryfilename>] [-b <indexfile>]\n"
        "Description:\n"
        "\t --help \t\t  print the help message\n"
        "\t -i <indexfile> \t  index construction and output to specified file\n"
        "\t -l <layout> \t\t  set the index layout scheme (0=In-cache, 1=Out-of-cache), default is 1\n"
        "\t -q <queryfilename> \t set the query file\n"
        "\nNote: Kp and Kt parameters are hardcoded in Graph.h and cannot be set via command line.\n"
        "      To change Kp or Kt, modify the constants in Graph.h and recompile the program.\n"
    << endl;
}

void output_index(CRIndex* reach, LandBitIndex* pll, Topo* topo,const string& filename) {
    ofstream output(filename, ios::out | ios::binary);
    assert(pll->Pb_in[0].size() == Kp && reach->reach_bit[0].size() == Kt);
    for (int i = 0; i < reach->reach_bit.size(); i++) {
	output.write((char *)&(reach->reach_bit[i]), sizeof(reach->reach_bit[i]));
        output.write((char *)&(pll->Pb_in[i]), sizeof(pll->Pb_in[i]));
        output.write((char *)&(pll->Pb_out[i]), sizeof(pll->Pb_out[i]));
        output.write((char *)&(topo->TopoOrder[i]), sizeof(topo->TopoOrder[i]));
        output.write((char *)&(topo->RevTopoOrder[i]), sizeof(topo->RevTopoOrder[i]));
    }
}

int main(int argc, char* argv[]){
	Graph g;
	ifstream input;
	string queryfilename;
	string filename;
    string queryIndex;
    string Indexfilename;
	clock_t start, end;
    // Default parameters
    int Layout = 1;                 // out of cache layout           
	bool IndexConstruction=false;
	bool Query=false;

    // Arg parsing
	if(argc==1){
	    usage();
	    exit(0);
	}
	int i=1;
	while(i<argc){
		if (strcmp("--help", argv[i]) == 0) {
			usage();
			exit(0);
		}
		else if (strcmp("-i", argv[i]) == 0) {
			i++;
			IndexConstruction = true;
			Indexfilename = argv[i++];
		}
		else if (strcmp("-q", argv[i]) == 0) {
			i++;
			Query = true;
			queryfilename=argv[i++];
		}
		else if (strcmp("-l", argv[i]) == 0) {
			i++;
			Layout = atoi(argv[i++]);
		}
		else {
			filename = argv[i++];
		}
	}
    // reading Graph File
	start=clock();
    // cout<< "Reading graph file: " << filename << endl;
	input.open(filename.c_str());
	if(!input.is_open()){
		cout << "Fail to open the file." << endl;
		exit(0);
	}
	g.readGraph(input);
	input.close();
	end=clock();

	if (IndexConstruction) {
		cout<<"Kp: " << Kp <<"\tKt: "<< Kt<<endl;
		double indexTime = 0.0, tmp = 0.0;
		struct timeval querystart, queryend;
		// cout << "***********Topo***********\n";
		gettimeofday(&querystart, NULL);
		Topo* topo = new Topo(&g);
		gettimeofday(&queryend, NULL);
		tmp = (queryend.tv_sec - querystart.tv_sec)*1000.0 + (queryend.tv_usec - querystart.tv_usec)*1.0/1000.0;
		indexTime += tmp;
		cout<< "Topo construction time(ms): " << tmp << endl;

		// cout << "***********LandBit***********\n";
		gettimeofday(&querystart, NULL);
		LandBitIndex* pll = new LandBitIndex(&g);
		gettimeofday(&queryend, NULL);
		tmp = (queryend.tv_sec - querystart.tv_sec)*1000.0 + (queryend.tv_usec - querystart.tv_usec)*1.0/1000.0;
		indexTime += tmp;
		cout<< "Landbit construction time(ms): " << tmp << endl;
		
		// cout << "***********CR***********\n";
		gettimeofday(&querystart, NULL);
		CRIndex* CR = new CRIndex(&g, pll, topo);
		gettimeofday(&queryend, NULL);
		tmp = (queryend.tv_sec - querystart.tv_sec)*1000.0 + (queryend.tv_usec - querystart.tv_usec)*1.0/1000.0;
		indexTime += tmp;
		cout<< "CR construction time(ms): " << tmp << endl;

		// cout << "********out_put*********\n";
		output_index(CR, pll, topo, Indexfilename); 
		cout << "Index time(s): " << indexTime/1000 << endl;
		cout<<"index Size(MB): " << 1.0 * (2 * sizeof(uint16_t) + 2 * Kp / 8 + Kt / 8 ) * g.vsize / (1024 * 1024) << endl;
		// return 0;
	}
	if(Query){
		if(Layout) g.QueryTest1(queryfilename, Indexfilename);
		else g.QueryTest0(queryfilename, Indexfilename);
	}
    return 0;
}

