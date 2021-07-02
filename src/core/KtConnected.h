#pragma once
#include "KtBfsIter.h"

// ͼ����ͨ����

template<typename GRAPH>
class KtConnected
{
	static_assert(!GRAPH::isDigraph(), "KtConnected cannot instantiated with Digraph.");

public:
	KtConnected(const GRAPH& g) 
		: count_(0),
		  cc_(g.order(), -1) {

		KtBfsIter<GRAPH, true> bfs(g, 0);
		unsigned id(0);
		for (; !bfs.isEnd(); ++bfs) {
			if (bfs.from() == -1)
				++id;
			cc_[*bfs] = id;
		}

		count_ = id;
	}


	// ������ͨ��������
	auto count() const { return count_; }

	// ����v�Ͷ���w�Ƿ���ͨ
	bool reachable(unsigned v, unsigned w) const {
		return cc_[v] == cc_[w];;
	}

	// ���ؽڵ�v������ͨ������id, 0 <= id < count().
	unsigned operator[](unsigned v) const {
		return cc_[v];
	}


private:
	unsigned count_; // ��ͨ����������
	std::vector<unsigned> cc_; // cc_[i]��ʾ����i��Ӧ����ͨ�������
};
