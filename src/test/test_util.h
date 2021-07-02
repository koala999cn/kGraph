#pragma once
#include <fstream>
#include <string>


// dump graph "g" to file "path"
template<typename GRAPH>
void dump(const GRAPH& g, const char* path, bool dumpValue)
{
	std::ofstream of(path);

	auto V = g.order();
	for (unsigned v = 0; v < V; v++) {
		typename GRAPH::adj_vertex_iter iter(g, v);
		std::string s = std::to_string(v);
		s += ", ";
		bool empty_line(true);
		for (; !iter.isEnd(); ++iter) {
			if (!g.isDigraph() && *iter < v)
				continue; // ��������ͼ��ֻ��ʾv <= w�ı�

			of << '(' << s << *iter;
			if (dumpValue) of << ", " << iter.value();
			of << ") ";
			empty_line = false;
		}

		if (!empty_line)
			of << '\n';
	}
}


// ����ʧ��ʱ���øú�����ӡ��Ϣ���˳�����
template<typename GRAPH>
void test_failed(const GRAPH& g, bool detail = false)
{
	printf("  > :( failed\n");
	printf("   dumping failed graph to 'graph_dump.txt'...\n");
	dump(g, "graph_dump.txt", detail);
	printf("press any key to exit.\n");
	fflush(stdout);
	getchar();
	exit(1);
}



// ����[x0, x1]����������
double rand(double x0, double x1);


// ��p�ĸ��ʷ���true.
// 0 <= p <= 1
bool rand_p(double p);


template<typename GRAPH>
GRAPH randGraph(unsigned V, unsigned E)
{
	GRAPH g(V);
	double p = static_cast<double>(E) / (V * V);
	if (!g.isDigraph()) p *= 2; // ��������ͼ���ߵ����ɸ��ʷ���
	for (unsigned i = 0; i < V; i++) {
		unsigned jMax = g.isDigraph() ? V : i + 1;
		for (unsigned j = 0; j < jMax; j++)
			if (rand_p(p)) {
				int r = 0;
				while (r == 0) r = rand();
				using value_type = typename GRAPH::value_type;
				value_type val(std::is_floating_point<value_type>::value ? value_type(r) / RAND_MAX : r);
				g.addEdge(i, j, val);
			}
	}

	return g;
}


#include "../core/KtConnected.h"
// ��ͼg�Ļ��������ӱߣ��Ա�֤gΪ��ͨͼ
template<typename GRAPH>
void makeConnect(GRAPH& g)
{
	KtConnected<GRAPH> cc(g);
	if (cc.count() > 1) {
		std::vector<unsigned> v(cc.count());
		unsigned cnt(1);
		for (unsigned i = 0; i < g.order(); i++) {
			if (cnt == cc[i]) {
				v.push_back(i); // ѹ��ÿ����ͨ�����ĵ�1������
				if (cnt++ == cc.count())
					break;
			}
		}

		for (unsigned i = 1; i < v.size(); i++)
			g.addEdge(v[i - 1], v[i]);

		assert(g.isConnected());
	}
}



// �ж�����ͼg1, g2�Ƿ����
template<typename G1, typename G2>
bool isSame(const G1& g1, const G2& g2)
{
	if (g1.order() != g2.order() || g1.size() != g2.size())
		return false;

	for (unsigned v = 0; v < g1.order(); v++)
		for (unsigned w = 0; w < g1.order(); w++)
			if (g1.getEdge(v, w) != g2.getEdge(v, w))
				return false;

	return true;
}


bool almostEqual(double x1, double x2, double tol = 1e-10);


// dump the pathR to string
std::string format_pathR(const std::vector<unsigned>& pathR);
