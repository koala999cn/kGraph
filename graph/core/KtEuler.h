#pragma once
#include <vector>
#include "KtGreedyIter.h"
#include "../util/is_connected.h"
#include "../util/degree.h"


// ŷ��ͼ�ж�����⣬ʵ��

/* 
 * �ж�ԭ��
 * 
 * A finite(undirected) connected graph is an Euler graph if and
 * only if exactly two vertices are of odd degree or all vertices are of even degree.
 * In the latter case, every Euler path of the graph is a cycle, and in the former
 * case, none is.
 *
 * A finite digraph is an Euler digraph if and only if its underlying
 * graph is connected and one of the following two conditions holds:
 * (i) There is one vertex a such that outdegree(a) = indegree(a)+1,
 * and another vertex b such that outdegree(b)+1 = indegree(b),
 * while for every other vertex v, outdegree(v) = indegree(v).
 * (ii) For every vertex v, outdegree(v) = indegree(v).
 * In the former case, every directed Euler path starts at a and ends in b. In the
 * latter, every directed Euler path is a directed Euler cycle.
 *
 */

 /*
  * ����㷨��Hierholzer�㷨(�����·��)�����Ӷ�O(V+E).
  *
  * �㷨���̣���������ջ��curPath��cycle����
  *  ��1��ѡ����һ����Ϊ��㣬��ջcurPath������������ʶ��㣬�������ı߶�ɾ���������Ķ�����ջcurPath��
  *  ��2�������ǰ����û�����ڱߣ��򽫸ö����curPath��ջ��cycle��
  *  ��3��cycleջ�еĶ����ջ˳�򣬾��Ǵ���������ŷ����·��
  *
  */


// ʹ��KtGreedyIterģ��ʵ��Hierholzer�㷨
// TODO: more test

template<typename GRAPH>
class KtEuler
{
public:
	KtEuler(const GRAPH& g) : g_(g) {}

	// test iff has euler-cycle or euler-path
	// ����0��ʾ��ŷ��ͼ������1��ʾ��ŷ����������2��ʾ��ŷ��·��
	// @oddVertex: ����nullptr���򷵻ص�һ����ȶ���
	int test(unsigned* oddVertex = nullptr) const {
		if (!is_connected(g_))
			return 0;

		int odd(0); // ��ȶ�����
		for (unsigned v = 0; v < g_.order(); v++) {
			if (degree(g_, v) % 2) {
				++odd;
				if (oddVertex) {
					*oddVertex = v;
					oddVertex = nullptr;
				}
			}
			if (odd > 2) return 0;
		}

		assert(odd != 1); // ����ͼ������ֻ��1����ȶ���

		return odd == 0 ? 1 : 2;
	}


	std::vector<unsigned> getCycle(unsigned startVertex = 0) const {
		std::vector<unsigned> cycle;
		KtGreedyIter<const GRAPH, false, true> dfs(g_, startVertex);
		for (; !dfs.isEnd(); ++dfs) {
			if (dfs.isPopping())
				cycle.push_back(*dfs);
		}

		if constexpr (GRAPH::isDigraph())
			std::reverse(cycle.begin(), cycle.end());

		return cycle;
	}


	std::vector<unsigned> getPath(unsigned startVertex) const {
		assert(degree(g_, startVertex) % 2); // startVertexΪ��ȶ���
		return getCycle(startVertex);
	}

private:
	const GRAPH& g_;
};
