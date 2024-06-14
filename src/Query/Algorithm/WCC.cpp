#include "PathQueryHandler.h"

using namespace std;

/**
	Compute and return the weakly connected components of the graph (when undirected,
	equivalent to the connected components).

	@param directed if false, treat all edges in the graph as bidirectional.
	@param pred_set the set of edge labels allowed.
	@return a vector of vectors, each containing the IDs of the vertices in a WCC.
**/
vector<vector<int>> PathQueryHandler::WCC(const std::vector<int> &pred_set)
{
	/*mark wcc*/
	unordered_map<int, int> vid2wccid;
	int wccid = 0;
	vector<int> wccsz;

	// loop each vertex for bfs start vertex

	// vector<unsigned> *id2vid = csr->id2vid;
	// unsigned pdnum = csr->pre_num;
	// for(unsigned pd=0; pd<pdnum; ++pd){
	//	for(int vertexid : id2vid[pd]){
	int vertex_num = getVertNum();
	for (int vertexid = 0; vertexid < vertex_num; ++vertexid)
	{
		if (vid2wccid.count(vertexid) == 0)
		{
			// cout << "start vertexid: " << vertexid << endl;

			unordered_set<int> visited; // visited in current wcc
			// mark when pushing into queue: to avoid one same vertex being added twice in the same level
			queue<int> q;
			int uid = vertexid; // current vertex
			q.push(uid);
			visited.insert(uid);
			vid2wccid[uid] = wccid;
			while (q.empty() == 0)
			{
				uid = q.front();
				q.pop();

				// directed or not, all neighbors
				for (int pred : pred_set)
				{
					int sz = getInSize(uid, pred), vid;
					for (int j = 0; j < sz; j++)
					{
						vid = getInVertID(uid, pred, j);
						if (visited.count(vid) == 0)
						{
							q.push(vid);
							visited.insert(vid);
							vid2wccid[vid] = wccid;
						}
					}
					sz = getOutSize(uid, pred);
					for (int j = 0; j < sz; j++)
					{
						vid = getOutVertID(uid, pred, j);
						if (visited.count(vid) == 0)
						{
							q.push(vid);
							visited.insert(vid);
							vid2wccid[vid] = wccid;
						}
					}
				}
			}
			++wccid;
			// cout << "current found WCC count: " << wccid << endl;

			// cout << "new wcc (sz:" << visited.size() << "): ";
			// for (auto v : visited)
			// {
			// 	cout << v << " ";
			// }
			// cout << endl;

			wccsz.push_back(visited.size());
		}
	}
	//}

	/*vid2wccid to result*/
	// already know the exact size of each wcc: utilize this to skip the "double alloc and copy" growing stage of STL vector
	vector<vector<int>> result(wccid); // call default constructor for each wcc vector, each 3 pointer, no further memory alloc
	for (int i = 0; i < wccid; ++i)
	{
		// reserve: Increase the capacity of the vector (the total number of elements that the vector can hold WITHOUT requiring reallocation)
		result[i].reserve(wccsz[i]);
	}
	for (auto pr : vid2wccid)
	{
		result[pr.second].push_back(pr.first);
	}

	// will call move constructor when construction return val
	return std::move(result);
}