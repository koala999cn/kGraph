#pragma once
#include <vector>


// ��adj_vertex_iter�Ķ��η�װ
// ��Ҫ���������ڽӶ�����������ϣ��ṩһ�������plus������Ϊ�����ڽӶ���
// ���øù���Ϊÿ������ģ��һ��������Ի�
template<typename GRAPH>
class KtAdjIterPlus
{
public:
	using iter_type = std::conditional_t<std::is_const_v<GRAPH>, 
		typename GRAPH::const_adj_iter, typename GRAPH::adj_iter>;

	using vertex_index_t = typename GRAPH::vertex_index_t;
	using edge_type = typename GRAPH::edge_type;
	using const_edge_ref = decltype(std::declval<iter_type>().edge());
	using plus_type = std::pair<vertex_index_t, edge_type>;


	KtAdjIterPlus(GRAPH& g, vertex_index_t v, const plus_type& plus) 
		: iter_(g, v), plus_(plus), plusVisited_(false) {}

	vertex_index_t operator*() const { 
		return plusVisited_ ? *iter_ : plus_.first;
	}

	const_edge_ref edge() const {
		return plusVisited_ ? iter_.edge() : plus_.second;
	}


	void operator++() { 
		if (plusVisited_)
			++iter_;
		else
			plusVisited_ = true;		
	}

	bool isEnd() const { return plusVisited_ && iter_.isEnd(); }

private:
	iter_type iter_;
	const plus_type& plus_;
	bool plusVisited_;
};