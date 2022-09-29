// TODO: Convert all IDs to unsigned to incorporate more data?

#include "../Database/CSR.h"
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <assert.h>
#include <unordered_set>

#ifndef _QUERY_PATH_H
#define _QUERY_PATH_H


/**
 * A custom vector. For some algorithms in PathQueryHandler.
 */
template <typename _T>
class iVector
{
public:
    unsigned int m_size;
    _T* m_data;
    unsigned int m_num;

    void free_mem()
    {
        delete[] m_data;
    }

    iVector()
    {
        m_size = 20;
        m_data = new _T[20];
        m_num = 0;
    }
    iVector( unsigned int n )
    {
        if ( n == 0 )
        {
            n = 20;
        }
        m_size = n;
        m_data = new _T[m_size];
        m_num = 0;
    }
    void push_back( _T d )
    {
        if ( m_num == m_size )
        {
            re_allocate( m_size*2 );
        }
        m_data[m_num] = d ;
        m_num++;
    }
    void push_back( const _T* p, unsigned int len )
    {
        while ( m_num + len > m_size )
        {
            re_allocate( m_size*2 );
        }
        memcpy( m_data+m_num, p, sizeof(_T)*len );
        m_num += len;
    }

    void re_allocate( unsigned int size )
    {
        if ( size < m_num )
        {
            return;
        }
        _T* tmp = new _T[size];
        memcpy( tmp, m_data, sizeof(_T)*m_num );
        m_size = size;
        delete[] m_data;
        m_data = tmp;
    }
    void Sort()
    {
        if ( m_num < 20 )
        {
            int k ;
            _T tmp;
            for ( int i = 0 ; i < m_num-1 ; ++i )
            {
                k = i ;
                for ( int j = i+1 ; j < m_num ; ++j )
                    if ( m_data[j] < m_data[k] ) k = j ;
                if ( k != i )
                {
                    tmp = m_data[i];
                    m_data[i] = m_data[k];
                    m_data[k] = tmp;
                }
            }
        }
        else sort( m_data, m_data+m_num );
    }
    void unique()
    {
        if ( m_num == 0 ) return;
        Sort();
        unsigned int j = 0;
        for ( unsigned int i = 0 ; i < m_num ; ++i )
            if ( !(m_data[i] == m_data[j]) )
            {
                ++j;
                if ( j != i ) m_data[j] = m_data[i];
            }
        m_num = j+1;
    }
    int BinarySearch( _T& data )
    {
        for ( int x = 0 , y = m_num-1 ; x <= y ; )
        {
            int p = (x+y)/2;
            if ( m_data[p] == data ) return p;
            if ( m_data[p] < data ) x = p+1;
            else y = p-1;
        }
        return -1;
    }
    void clean()
    {
        m_num = 0;
    }
    void assign( iVector& t )
    {
        m_num = t.m_num;
        m_size = t.m_size;
        delete[] m_data;
        m_data = t.m_data;
    }

    bool remove( _T& x )
    {
        for ( int l = 0 , r = m_num ; l < r ; )
        {
            int m = (l+r)/2;

            if ( m_data[m] == x )
            {
                m_num--;
                if ( m_num > m ) memmove( m_data+m, m_data+m+1, sizeof(_T)*(m_num-m) );
                return true;
            }
            else if ( m_data[m] < x ) l = m+1;
            else r = m;
        }
        return false;
    }

    void sorted_insert( _T& x )
    {
        if ( m_num == 0 )
        {
            push_back( x );
            return;
        }

        if ( m_num == m_size ) re_allocate( m_size*2 );

        int l,r;

        for ( l = 0 , r = m_num ; l < r ; )
        {
            int m = (l+r)/2;
            if ( m_data[m] < x ) l = m+1;
            else r = m;
        }

        if ( m_num > l )
        {
            memmove( m_data+l+1, m_data+l, sizeof(_T)*(m_num-l) );
        }
        m_num++;
        m_data[l] = x;
    }

    bool remove_unsorted( _T& x )
    {
        for ( int m = 0 ; m < m_num ; ++m )
        {
            if ( m_data[m] == x )
            {
                m_num--;
                if ( m_num > m ) memcpy( m_data+m, m_data+m+1, sizeof(_T)*(m_num-m) );
                return true;
            }
        }
        return false;
    }

    _T& operator[]( unsigned int i )
    {
        return m_data[i];
    }
    //close range check for [] in iVector if release

};

/**
 * A custom map. For some algorithms in PathQueryHandler.
 */
template <typename _T>
struct iMap
{
    _T* m_data;
    int m_num;
    int cur;
    iVector<int> occur;
    _T nil;
    iMap()
    {
        m_data = NULL;
        m_num = 0;
    }
    iMap(int size){
        initialize(size);
    }
    void free_mem()
    {
        delete[] m_data;
        occur.free_mem();
    }

    void initialize( int n )
    {
        occur.re_allocate(n);
        occur.clean();
        m_num = n;
        if ( m_data != NULL )
            delete[] m_data;
        m_data = new _T[m_num];
        for ( int i = 0 ; i < m_num ; ++i )
            m_data[i] = nil;
        cur = 0;
    }
    void clean()
    {
        for ( int i = 0 ; i < occur.m_num ; ++i )
        {
            m_data[occur[i]] = nil;
        }
        occur.clean();
        cur = 0;
    }

    //init keys 0-n, value as 0
    void init_keys(int n){
        occur.re_allocate(n);
        occur.clean();
        m_num = n;
        if ( m_data != NULL )
            delete[] m_data;
        m_data = new _T[m_num];
        for ( int i = 0 ; i < m_num ; ++i ){
            m_data[i] = 0;
            occur.push_back( i );
            cur++;
        }
    }
    //reset all values to be zero
    void reset_zero_values(){
        memset( m_data, 0.0, m_num*sizeof(_T) );
    }

    void reset_one_values(){
        for ( int i = 0 ; i < m_num ; ++i )
            m_data[i] = 1.0;
    }

    _T get( int p )
    {
        return m_data[p];
    }
    _T& operator[](  int p )
    {
        return m_data[p];
    }
    void erase( int p )
    {
        m_data[p] = nil;
        cur--;
    }
    bool notexist( int p )
    {
        return m_data[p] == nil ;
    }
    bool exist( int p )
    {
        return !(m_data[p] == nil);
    }
    void insert( int p , _T d )
    {
        if ( m_data[p] == nil )
        {
            occur.push_back( p );
            cur++;
        }
        m_data[p] = d;
    }
    void inc( int p )
    {
        m_data[p]++;
    }
    void inc( int p , int x )
    {
        m_data[p] += x;
    }
    void dec( int p )
    {
        m_data[p]--;
    }
    //close range check when release!!!!!!!!!!!!!!!!!!!!
};

class PathQueryHandler
{
private:
	CSR *csr;
	std::unordered_map<int, std::set<int> > distinctInEdges;
	std::unordered_map<int, std::set<int> > distinctOutEdges;
	int cacheMaxSize;
	int n, m;	// #vertices, #edges
public:
	PathQueryHandler(CSR *_csr);
	
	~PathQueryHandler();

	void inputGraph(std::string filename);	// Read in a graph FOR TESTING. Graph file format:
						// First line: #vertices #labels
						// Each following line: u v label
	void printCSR();	// Feel free to modify this for testing
	// void generateQueries(int queryType, bool directed, int numQueries, vector<pair<int, int>, int>& queries);

	// Labeled graph interface: edge (u, v, pred)

	int getVertNum();	// Get total number of vertices
	int getEdgeNum();	// Get total number of edges
    int getSetEdgeNum(const std::vector<int> &pred_set);    // TODO: Get the number of edges labeled by preds in pred_set
	int getInIndexByID(int vid, int pred);	// Get vertice's index in csr[1] offset_list
	int getInSize(int vid, int pred);	// Get the number of in-neighbors of vert
						// linked by edges labeled by pred
	int getInVertID(int vid, int pred, int pos);	// Get the pos-th in-neighbor of vert
						// linked by an edge labeled by pred
	int getInVertID(int vid, int pos);	// Get in-neighbor regardless of pred;
						// corresponds to getTotalInSize with distinct
	int getSetInSize(int vid, const std::vector<int> &pred_set);	// Get number of in-neighbors by pred_set
	int getTotalInSize(int vid, bool distinct);	// Get the total number of in-neighbors of vert
						// distinct will eliminate repetitive occurrences of same in-neighbor with different label edges

	int getOutIndexByID(int vid, int pred);
	int getOutSize(int vid, int pred);
	int getOutVertID(int vid, int pred, int pos);
	int getOutVertID(int vid, int pos);
	int getSetOutSize(int vid, const std::vector<int> &pred_set);
	int getTotalOutSize(int vid, bool distinct);
	void dfs(std::map<int, std::vector<int> > &route, std::map<int, bool> &vis, int q, int v, const std::vector<int> pred_set, std::vector<int> &ans, bool &finished);

	// Path query evaluation functions
	
	std::vector<int> simpleCycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> cycle(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> shortestPath(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	std::vector<int> shortestPath0(int uid, int vid, bool directed, const std::vector<int> &pred_set);
	bool kHopReachable(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
	bool kHopReachableTest(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
	std::vector<int> kHopReachablePath(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
    std::vector<std::vector<int>> kHopEnumeratePath(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
    int bc_dfs(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set, std::vector<std::pair<int, int>> &s, std::vector<int> &bar, std::vector<std::vector<int>> &paths);

	void SSPPR(int uid, int retNum, int k, const std::vector<int> &pred_set, std::vector< std::pair<int ,double> > &topkV2ppr);
    long long triangleCounting(bool directed, const std::vector<int> &pred_set);
    double closenessCentrality(int uid, bool directed, const std::vector<int> &pred_set);
    std::vector<int> bfsCount(int uid, bool directed, const std::vector<int> &pred_set);

    std::vector<std::pair<std::pair<int, int>, int>> kHopSubgraph(int uid, int vid, bool directed, int k, const std::vector<int> &pred_set);
	
    std::vector<int> BFS(int uid, bool directed, const std::vector<int> &pred_set, bool forward=true);
    
	std::string getPathString(std::vector<int> &path_set);
private:
    // Helper functions for SSPPR
    void compute_ppr_with_reserve(std::pair<iMap<double>, iMap<double>> &fwd_idx, std::unordered_map<int, double> &v2ppr);
    void forward_local_update_linear_topk(int s, double& rsum, double rmax, double lowest_rmax, \
        std::vector<std::pair<int, int>>& forward_from, std::pair<iMap<double>, iMap<double>> &fwd_idx, \
        const std::vector<int> &pred_set, double alpha, int k);
    void compute_ppr_with_fwdidx_topk_with_bound(double check_rsum, std::pair<iMap<double>, iMap<double>> &fwd_idx, \
        std::unordered_map<int, double> &v2ppr, double delta, double alpha, double threshold, \
        const std::vector<int> &pred_set, double pfail, double &zero_ppr_upper_bound, double omega, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds);
    void set_ppr_bounds(std::pair<iMap<double>, iMap<double>> &fwd_idx, double rsum, long total_rw_num, \
        std::unordered_map<int, double> &v2ppr, double pfail, double &zero_ppr_upper_bound, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds);
    inline double calculate_lambda(double rsum, double pfail, double upper_bound, long total_rw_num);
    inline int random_walk(int start, double alpha, const std::vector<int> &pred_set);
    double kth_ppr(std::unordered_map<int, double> &v2ppr, int retNum);
    bool if_stop(int retNum, double delta, double threshold, double epsilon, iMap<int> &topk_filter, \
        iMap<double> &upper_bounds, iMap<double> &lower_bounds, std::unordered_map<int, double> &v2ppr);
};

#endif //_QUERY_PATH_H