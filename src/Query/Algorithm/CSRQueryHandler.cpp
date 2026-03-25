#include "CSRQueryHandler.h"

using namespace std;

CSRQueryHandler::CSRQueryHandler(std::shared_ptr<CSR[]>& _csr)
{
	if (_csr)
		csr = _csr;
	else
    {
        csr = std::shared_ptr<CSR[]>(new CSR[2], std::default_delete<CSR[]>());
    }
	cacheMaxSize = 10000;
	n = -1;
	m = -1;
	srand(time(NULL));
}

void CSRQueryHandler::updateCsr(std::shared_ptr<CSR[]>& _csr)
{
	if (_csr)
		csr = _csr;
	else
    {
        csr = std::shared_ptr<CSR[]>(new CSR[2], std::default_delete<CSR[]>());
    }
	cacheMaxSize = 10000;
	n = -1;
	m = -1;
	srand(time(NULL));
}

CSRQueryHandler::~CSRQueryHandler()
{
}

int CSRQueryHandler::getPreNum()
{
	return csr[1].pre_num;
}

unsigned int CSRQueryHandler::getVertNum()
{
	// if (n != -1)
	// 	return n; // Only consider static graphs for now
	// set<int> vertices;
	// for (int j = 0; j < 2; j++)
	// {
	// 	for (unsigned i = 0; i < csr[j].pre_num; i++)
	// 		vertices.insert(csr[j].adjacency_list[i].begin(), csr[j].adjacency_list[i].end());
	// }
	// n = vertices.size();
	// return n;

	// save vertNum to CSR.n when database loadCSR 
	return csr[1].n;
}

unsigned int CSRQueryHandler::getEdgeNum()
{
	// if (m != -1)
	// 	return m; // Only consider static graphs for now
	// int ret = 0;
	// for (unsigned i = 0; i < csr[1].pre_num; i++) // Same as summing that of csr[0]
	// 	ret += csr[1].adjacency_list[i].size();
	// m = ret;
	// return m;

	// save edgeNum to CSR.m when database loadCSR 
	return csr[1].m;
}

int CSRQueryHandler::getSetEdgeNum(const vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += csr[1].adjacency_list[pred].size();
	return ret;
}

int CSRQueryHandler::getInIndexByID(int vid, int pred)
{
	if (csr[1].vid2id[pred].find(vid) != csr[1].vid2id[pred].end())
		return csr[1].vid2id[pred][vid];
	else
		return -1;
}

int CSRQueryHandler::getInSize(int vid, int pred)
{
	int vIndex = getInIndexByID(vid, pred);
	if (vIndex == -1) // This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == static_cast<int>(csr[1].offset_list[pred].size()) - 1)
		return csr[1].adjacency_list[pred].size() - csr[1].offset_list[pred][vIndex];
	else
		return csr[1].offset_list[pred][vIndex + 1] - csr[1].offset_list[pred][vIndex];
}

int CSRQueryHandler::getInVertID(int vid, int pred, int pos)
{
	if (pos >= getInSize(vid, pred))
		return -1;
	int offset = csr[1].offset_list[pred][getInIndexByID(vid, pred)];
	return csr[1].adjacency_list[pred][offset + pos];
}

int CSRQueryHandler::getInVertID(int vid, int pos)
{
	if (distinctInEdges.find(vid) == distinctInEdges.end())
		getTotalInSize(vid, true); // Load into cache

	if (pos < static_cast<int>(distinctInEdges[vid].size()))
		return *next(distinctInEdges[vid].begin(), pos);
	else
		return -1;
}

int CSRQueryHandler::getSetInSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getInSize(vid, pred);
	return ret;
}

int CSRQueryHandler::getTotalInSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < static_cast<int>(csr[1].pre_num); i++)
			ret += getInSize(vid, i);
	}
	else
	{
		if (distinctInEdges.find(vid) == distinctInEdges.end())
		{
			if (static_cast<int>(distinctInEdges.size()) == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctInEdges.erase(next(distinctInEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctInEdges[vid] = set<int>();
			for (int pred = 0; pred < static_cast<int>(csr[1].pre_num); pred++)
			{
				int vIndex = getInIndexByID(vid, pred);
				if (vIndex == -1) // This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == static_cast<int>(csr[1].offset_list[pred].size()) - 1 && csr[1].adjacency_list[pred].size() > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]),
												csr[1].adjacency_list[pred].end());
				else if (csr[1].offset_list[pred][vIndex + 1] > csr[1].offset_list[pred][vIndex])
					distinctInEdges[vid].insert(next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex]),
												next(csr[1].adjacency_list[pred].begin(), csr[1].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctInEdges[vid].size();
	}
	return ret;
}

int CSRQueryHandler::getOutIndexByID(int vid, int pred)
{
	if (csr[0].vid2id[pred].find(vid) != csr[0].vid2id[pred].end())
		return csr[0].vid2id[pred][vid];
	else
		return -1;
}

int CSRQueryHandler::getOutSize(int vid, int pred)
{
	int vIndex = getOutIndexByID(vid, pred);
	if (vIndex == -1) // This vertex does not participate in this pred's relations
		return 0;
	else if (vIndex == static_cast<int>(csr[0].offset_list[pred].size()) - 1)
		return csr[0].adjacency_list[pred].size() - csr[0].offset_list[pred][vIndex];
	else
		return csr[0].offset_list[pred][vIndex + 1] - csr[0].offset_list[pred][vIndex];
}

int CSRQueryHandler::getOutVertID(int vid, int pred, int pos)
{
	if (pos >= getOutSize(vid, pred))
		return -1;
	int offset = csr[0].offset_list[pred][getOutIndexByID(vid, pred)];
	return csr[0].adjacency_list[pred][offset + pos];
}

int CSRQueryHandler::getOutVertID(int vid, int pos)
{
	if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		getTotalOutSize(vid, true); // Load into cache

	if (pos < static_cast<int>(distinctOutEdges[vid].size()))
		return *next(distinctOutEdges[vid].begin(), pos);
	else
		return -1;
}

int CSRQueryHandler::getSetOutSize(int vid, const std::vector<int> &pred_set)
{
	int ret = 0;
	for (int pred : pred_set)
		ret += getOutSize(vid, pred);
	return ret;
}

int CSRQueryHandler::getTotalOutSize(int vid, bool distinct)
{
	int ret = 0;
	if (!distinct)
	{
		for (int i = 0; i < static_cast<int>(csr[1].pre_num); i++)
			ret += getOutSize(vid, i);
	}
	else
	{
		if (distinctOutEdges.find(vid) == distinctOutEdges.end())
		{
			if (static_cast<int>(distinctOutEdges.size()) == cacheMaxSize)
			{
				int replacement = rand() % cacheMaxSize;
				distinctOutEdges.erase(next(distinctOutEdges.begin(), replacement));
				// cout << "distinctInEdges replaced entry " << replacement << endl;
			}

			distinctOutEdges[vid] = set<int>();
			for (int pred = 0; pred < static_cast<int>(csr[1].pre_num); pred++)
			{
				int vIndex = getOutIndexByID(vid, pred);
				if (vIndex == -1) // This vertex does not participate in this pred's relations
					continue;
				else if (vIndex == static_cast<int>(csr[0].offset_list[pred].size()) - 1 && csr[0].adjacency_list[pred].size() > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]),
												 csr[0].adjacency_list[pred].end());
				else if (csr[0].offset_list[pred][vIndex + 1] > csr[0].offset_list[pred][vIndex])
					distinctOutEdges[vid].insert(next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex]),
												 next(csr[0].adjacency_list[pred].begin(), csr[0].offset_list[pred][vIndex + 1]));
			}
		}
		ret = distinctOutEdges[vid].size();
	}
	return ret;
}

void CSRQueryHandler::inputGraph(string filename)
{
	// ifstream infile(filename.c_str());
	ifstream infile(filename);

	int n, numLabel;
	infile >> n >> numLabel;
	csr[0].init(numLabel);
	csr[1].init(numLabel);

	int **indegree = new int *[numLabel];
	int **outdegree = new int *[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		indegree[i] = new int[n];
		memset(indegree[i], 0, n * sizeof(int));
		outdegree[i] = new int[n];
		memset(outdegree[i], 0, n * sizeof(int));
	}

	int from, to, label;
	while (infile >> from >> to >> label)
	{
		outdegree[label][from]++;
		indegree[label][to]++;
	}
	int ***inAdjList = new int **[numLabel];
	int ***outAdjList = new int **[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		inAdjList[i] = new int *[n];
		outAdjList[i] = new int *[n];
		for (int j = 0; j < n; j++)
		{
			inAdjList[i][j] = new int[indegree[i][j]];
			outAdjList[i][j] = new int[outdegree[i][j]];
		}
	}
	int **pointer_in = new int *[numLabel];
	int **pointer_out = new int *[numLabel];
	for (int i = 0; i < numLabel; i++)
	{
		pointer_in[i] = new int[n];
		memset(pointer_in[i], 0, n * sizeof(int));
		pointer_out[i] = new int[n];
		memset(pointer_out[i], 0, n * sizeof(int));
	}

	infile.clear();
	infile.seekg(0);
	infile >> n >> numLabel;
	while (infile >> from >> to >> label)
	{
		outAdjList[label][from][pointer_out[label][from]] = to;
		pointer_out[label][from]++;
		inAdjList[label][to][pointer_in[label][to]] = from;
		pointer_in[label][to]++;
	}
	infile.close();

	int *pointer_outAdj = new int[numLabel];
	int *pointer_inAdj = new int[numLabel];
	memset(pointer_outAdj, 0, numLabel * sizeof(int));
	memset(pointer_inAdj, 0, numLabel * sizeof(int));
	for (int i = 0; i < numLabel; i++)
	{
		for (int j = 0; j < n; j++)
		{
			csr[0].id2vid[i].push_back(j);
			csr[0].vid2id[i][j] = j;
			csr[0].offset_list[i].push_back(pointer_outAdj[i]);
			for (int k = 0; k < outdegree[i][j]; k++)
			{
				csr[0].adjacency_list[i].push_back(outAdjList[i][j][k]);
				pointer_outAdj[i]++;
			}

			csr[1].id2vid[i].push_back(j);
			csr[1].vid2id[i][j] = j;
			csr[1].offset_list[i].push_back(pointer_inAdj[i]);
			for (int k = 0; k < indegree[i][j]; k++)
			{
				csr[1].adjacency_list[i].push_back(inAdjList[i][j][k]);
				pointer_inAdj[i]++;
			}
		}
	}

	for (int i = 0; i < numLabel; i++)
	{
		delete[] indegree[i];
		delete[] outdegree[i];
	}
	delete[] indegree;
	delete[] outdegree;
	for (int i = 0; i < numLabel; i++)
	{
		for (int j = 0; j < n; j++)
		{
			delete[] inAdjList[i][j];
			delete[] outAdjList[i][j];
		}
		delete[] inAdjList[i];
		delete[] outAdjList[i];
	}
	delete[] inAdjList;
	delete[] outAdjList;
	for (int i = 0; i < numLabel; i++)
	{
		delete[] pointer_in[i];
		delete[] pointer_out[i];
	}
	delete[] pointer_in;
	delete[] pointer_out;
	delete[] pointer_outAdj;
	delete[] pointer_inAdj;
}

void CSRQueryHandler::printCSR()
{
	cout << "----------OUT----------" << endl;
	csr[0].print();
	cout << endl;
	cout << "----------IN----------" << endl;
	csr[1].print();
}