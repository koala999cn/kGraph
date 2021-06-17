#pragma once
#include "KtDfsIter.h"
#include <assert.h>
#include <queue>


// ����Դ�㣨���Ϊ0�Ķ��㣩���е����������㷨
// ���������2�ֶ��壺
//   1.���±�ţ���DAG�������±�ŴӶ�ʹÿ�����ɽ�С��Ŷ���ָ��ϴ��Ŷ���
//   2.������������DAG������һ��ˮƽ���������������Ӷ�ʹ���б߾�����ָ����
// �������ֶ����ǵȼۺͻ���ģ�����֪��������ts�������±�ż��㷽�����£�tsI[ts[v]] = v
// ͬ��������֪���±��tsI���������������㷽�����£�ts[tsI[v]] = v
template<typename DAG>
class KtTopologySort
{
	static_assert(DAG::isDigraph(), "KtTopologySort must instantiated with Digraph.");

public:
    KtTopologySort(const DAG& dag) {
        assert(!dag.hasLoop());
        unsigned V = dag.order();
        ts_.resize(V, -1); tsI_.resize(V, -1);

        // �������������
        std::vector<unsigned> ins(V, 0);
        for(unsigned v = 0; v < V; v++) { 
            typename DAG::adj_vertex_iter iter(dag, v);
            while(!iter.isEnd()) {
                ins[*iter]++;
                ++iter;
            }
        }
        
        // ��Դ���������q
        std::queue<unsigned> q; // Դ�����
        for(unsigned v = 0; v < V; v++) 
            if(ins[v] == 0) q.push(v);
		assert(!q.empty());

        // ����FIFO˳�򣬶�Դ�������������
        for(unsigned i = 0; !q.empty(); i++) {
            unsigned v = q.front(); q.pop();
            ts_[i] = v; 
            tsI_[v] = i;
            typename DAG::adj_vertex_iter iter(dag, v);
			for (; !iter.isEnd(); ++iter)
                if(0 == --ins[*iter]) 
					q.push(*iter);
        }
    }

    // ������������
    unsigned operator[](unsigned v) const {
        return ts_[v];
    }
    
    // �������±��
    unsigned relabel(unsigned v) const {
        return tsI_[v];
    }

private:
    std::vector<unsigned> ts_, tsI_; 
};



// ����Dfs�㷨������������
// DFS�еĺ����ſ���Ϊ�κ�DAG�õ�һ������������
template<typename DAG>
class KtTopologySortInv
{
	static_assert(DAG::isDigraph(), "KtTopologySortInv must instantiated with Digraph.");

public:
    KtTopologySortInv(const DAG& dag) : dfs_(dag, 0) {
		//assert(dag.hasLoop());

        while(!dfs_.isEnd())
            ++dfs_;
        
        unsigned V = dag.order();
        popI_.resize(V);
		for (unsigned v = 0; v < V; v++) {
			assert(relabel(v) < V);
			popI_[relabel(v)] = v;
		}
    }


    unsigned operator[](unsigned v) const {
        return popI_[v];
    }
    
    unsigned relabel(unsigned v) const {
        return dfs_.popIndex(v);
    }


private:
    KtDfsIter<DAG, true> dfs_;
    std::vector<unsigned> popI_;
};