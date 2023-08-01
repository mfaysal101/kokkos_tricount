#ifndef CSRMAT_H
#define CSRMAT_H

#include<set>
#include<vector>
#include<fstream>

using namespace std;

typedef pair<vector<int>, vector<int>> CSR;
typedef std::pair<int, int> MyEdge;
typedef std::vector<MyEdge> MyEdgeList;
typedef std::vector<std::set<int>> ListOfList;


class CSRMat
{
public:
	int numrows, numcols, nnz;
	vector<int> rowptr;		//equivalent representation of IA
	vector<int> colids;		//equivalent representation of JA
	vector<int> values;		


	int getNumVertices(const MyEdgeList& edges)
	{
		int num = 0;

		#pragma omp parallel for reduction (max:num)
		for (int i = 0; i < edges.size(); i++)
		{
			num = std::max(num, 1 + std::max(edges[i].first, edges[i].second));
		}

		int numVertices = num;

		std::cout << "Total number of vertices:" << numVertices << std::endl;

		return numVertices;
	}

	ListOfList EdgeToAdjList(const MyEdgeList& edges)
	{
		auto start = std::chrono::high_resolution_clock::now();

		ListOfList adjlist(getNumVertices(edges));

		for (auto edge : edges)
		{
			adjlist[edge.first].insert(edge.second);
			adjlist[edge.second].insert(edge.first);
		}
	
		auto end = std::chrono::high_resolution_clock::now();
		double edge2adj_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

		return adjlist;
	}

	
	/*
	void sortInPlaceAdjList(ListOfList& adjlist)
	{
		auto start = std::chrono::high_resolution_clock::now();

		int len = adjlist.size();

		#pragma omp parallel for
		for (int i = 0; i < len; i++)
		{
			sort(adjlist[i].begin(), adjlist[i].end());
		}

		auto end = std::chrono::high_resolution_clock::now();
		double sorting_adj_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	}
	*/


	CSRMat()
	{
		numrows = 0;
		numcols = 0;
		nnz = 0;
	}

	CSRMat(string filename)
	{
		auto start = std::chrono::high_resolution_clock::now();
		int M, N, L;
		ifstream fin(filename);
		// Ignore headers and comments:
		while (fin.peek() == '%') fin.ignore(2048, '\n');
		// Read defining parameters:
		fin >> M >> N >> L;
		numrows = M;
		numcols = N;

		vector<vector<bool>> A_mat(M, vector<bool>(N, false));

		cout <<"M:"<<M<<" N:"<<N<<" L:"<<L<<endl;

		string line;

		while (std::getline(fin, line)) 
		{
			std::istringstream iss(line);
			int src, dst;
			if ((iss >> src >> dst))
			{
				if(src == dst)
				{
					continue;
				}
				A_mat[src][dst] = true;
				A_mat[dst][src] = true;
			}

    		}
		/*
		while(fin >> row >> col)
		{	
			A_mat[row][col] = true;
			A_mat[col][row] = true;
			cout<<"row:"<<row<<" col:"<<col<<endl;	
		}
		*/
		fin.close();
		rowptr.push_back(0);
		
		for(int i = 0; i < M; i++)
		{
			for(int j = 0; j < N; j++)
			{	
				
				if(A_mat[i][j])
				{
					colids.push_back(j);
					values.push_back(1);	
				}
						
			}
			rowptr.push_back(colids.size());
		}
		auto end = std::chrono::high_resolution_clock::now();
		double FileIOtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		printf("========Input graph read time:%0.9f===========\n", FileIOtime*(1e-9));
	}

	CSRMat(string filename, bool noMat)
	{
		auto start = std::chrono::high_resolution_clock::now();
		int M, N, L;
		ifstream fin(filename);
		// Ignore headers and comments:
		while (fin.peek() == '%') fin.ignore(2048, '\n');
		// Read defining parameters:
		fin >> M >> N >> L;
		numrows = M;
		numcols = N;
		MyEdgeList edgelist;

		string line;

		
		while (std::getline(fin, line)) 
		{
			std::istringstream iss(line);
			int src, dst;
			if ((iss >> src >> dst))
			{
				if(src == dst)
				{
					continue;
				}
				edgelist.push_back({src, dst});	
			}
    		}

		int numEdges = edgelist.size();

		
		std::cout << "Total number of edges:" << numEdges << std::endl;

		sort(edgelist.begin(), edgelist.end(), [](const std::pair<int, int>& edge1, std::pair<int, int>& edge2) {
			return (edge1.first < edge2.first) || (edge1.first == edge2.first && edge1.second < edge2.second);
		});
		
		ListOfList adjlist = EdgeToAdjList(edgelist);
		//sortInPlaceAdjList(adjlist);
		
		rowptr.push_back(0);
		for(auto adj: adjlist)
		{
			colids.insert(colids.end(), adj.begin(), adj.end());
			values.insert(values.end(), adj.size(), 1);
			rowptr.push_back(colids.size());
		}

		std::cout << "Coming to this point"<< std::endl;
	}

/*
	CSRMat(string filename)
	{
		auto start = std::chrono::high_resolution_clock::now();
		int M, N, L;
		ifstream fin(filename);
		// Ignore headers and comments:
		while (fin.peek() == '%') fin.ignore(2048, '\n');
		// Read defining parameters:
		fin >> M >> N >> L;
		numrows = M;
		numcols = N;
		int row, col;

		vector<vector<bool>> A_mat(M+1, vector<bool>(N+1, false));

		cout <<"M:"<<M<<" N:"<<N<<" L:"<<L<<endl;
		while(fin >> row >> col)
		{	
			A_mat[row][col] = true;
			A_mat[col][row] = true;
			cout<<"row:"<<row<<" col:"<<col<<endl;	
		}
		fin.close();
		printf("Reached this point\n");
		rowptr.push_back(0);

		for(int i = 0; i < A_mat.size(); i++)
		{
			for(int j = 0; j < A_mat[0].size(); j++)
			{
				if(A_mat[i][j])
				{
					colids.push_back(j);
					values.push_back(1);	
				}		
			}
			rowptr.push_back(colids.size());
		}
		auto end = std::chrono::high_resolution_clock::now();
		double FileIOtime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		printf("========Input graph read time:%0.9f===========\n", FileIOtime*(1e-9));
	}
*/
};


#endif
