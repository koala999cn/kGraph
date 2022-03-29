#pragma once
#include <algorithm>
#include <set>
#include "graph_traits.h"
#include "vertex_traits.h"
#include "edge_traits.h"
#include "../base/KtCondRange.h"
#include "KtAdjIter.h"


namespace kPrivate
{
	template<bool reshapable, bool immutable>
	struct graph_level {
		constexpr static const int value = 0;
	};

	template<bool reshapable>
	struct graph_level<reshapable, false> {
		constexpr static const int value = 1;
	};

	template<>
	struct graph_level<true, false> {
		constexpr static const int value = 2;
	};

	template<typename GRAPH>
	constexpr static const int graph_level_v = 
		graph_level<graph_traits<GRAPH>::reshapable, graph_traits<GRAPH>::immutable>::value;

	template<typename EDGE_TYPE>
	struct KpEdgeCompAllInOne {
		bool operator()(unsigned to, const EDGE_TYPE& e) const {
			return to < edge_traits<EDGE_TYPE>::to(e);
		}

		bool operator()(const EDGE_TYPE& e, unsigned to) const {
			return edge_traits<EDGE_TYPE>::to(e) < to;
		}

		bool operator()(const EDGE_TYPE& e1, const EDGE_TYPE& e2) const {
			return edge_traits<EDGE_TYPE>::to(e1) < edge_traits<EDGE_TYPE>::to(e2);
		}
	};
};


template<typename GRAPH_IMPL, bool digraph, bool multiEdges = false, bool alwaysSorted = false,
	int level = kPrivate::graph_level_v<GRAPH_IMPL>>
class KtGraph : public GRAPH_IMPL
{
public:
	using super_ = GRAPH_IMPL;

	// ��������
	using vertex_type = typename super_::vertex_type;
	using edge_type = typename super_::edge_type;

	using underly_vertex_t = typename graph_traits<GRAPH_IMPL>::underly_vertex_t;
	using underly_edge_t = typename graph_traits<GRAPH_IMPL>::underly_edge_t;

	using vertex_index_t = unsigned;


	// ���뺯��
	using super_::super_;
	using super_::order;
	using super_::size;

	constexpr static bool isDigraph() { return digraph; }
	constexpr static bool isDense() { return graph_traits<GRAPH_IMPL>::is_dense; }
	constexpr static bool isMultiEdges() { return multiEdges && !isDense(); }
	constexpr static bool isAlwaysSorted() { return alwaysSorted; }
	constexpr static bool hasVertex() { return !std::is_void_v<vertex_type>; }


	// ��ͼ
	bool isEmpty() const { return order() == 0; }

	// ƽ��ͼ
	bool isTrivial() const { return order() == 1; }

	// ��ͼ
	bool isNull() const { return size() == 0; }


	// ����v�ĳ��ȣ���v�ж���������
	unsigned outdegree(unsigned v) const { 
		if constexpr (vertexHasOutDegree_()) {
			return outdegree_(v);
		}
		else if constexpr (!isDense()) {
			return static_cast<unsigned>(outedges(v).size());
		}
		else {
			struct {
				bool operator()(const edge_type& e) {
					return super_::null_edge != e;
				}
			} pred;

			decltype(auto) r = outedges(v);
			return static_cast<unsigned>(std::count_if(r.begin(), r.end(), pred));
		}
	}


	// ���
	unsigned indegree(unsigned v) const {
		if constexpr (!isDigraph()) {
			return outdegree(v);
		}
		else {
			unsigned d(0);
			for (unsigned u = 0; u < order(); u++)
				d += static_cast<unsigned>(edges(u, v).size());
			return d;
		}
	}


	// ��. ע����������ͼ���Ի�����Ϊ2��1�����+1������
	unsigned degree(unsigned v) const {
		auto d = outdegree(v);
		if constexpr (isDigraph()) 
			d += indegree(v);
		return d;
	}

	auto edges(unsigned from, unsigned to) const {
		decltype(auto) r = super_::outedges(from);
		if constexpr (isDense()) {
			auto first = std::next(r.begin(), to);
			if (*first == super_::null_edge)
				return KtRange(first, 0);
			else
				return KtRange(first, 1);
		}
		else if constexpr (isAlwaysSorted()) {
			auto range = std::equal_range(r.begin(), r.end(), to, 
				kPrivate::KpEdgeCompAllInOne<underly_edge_t>{});
			return KtRange(range.first, range.second);
		}
		else { // ��Ҫ����ȫ�������ռ�<from, to>������multi-edges
			
			static_assert(has_to_v<underly_edge_t>, "edge missing property of 'to'");

			auto pred =	[to](typename decltype(r)::const_deref_type e) {
				    return to == edge_traits<underly_edge_t>::to(e); 
			};

			auto pos = std::find_if(r.begin(), r.end(), pred);
			if constexpr (!isMultiEdges()) {
				return KtRange(pos, pos == r.end() ? 0 : 1);
			}
			else {
				return KtCondRange<decltype(pos), decltype(pred)>{ pos, r.end(), pred };
			}
		}
	}


	bool hasEdge(unsigned from, unsigned to) const {
		return !edges(from, to).empty();
	}


	// ���Ե���ͼ��Ч
	template<bool dummy = !multiEdges, typename = std::enable_if_t<dummy>>
	const edge_type& getEdge(unsigned from, unsigned to) const {
		assert(hasEdge(from, to));
		auto edges = this->edges(from, to);
		return *edges;
	}


	// ���Զ��ͼ��Ч
	template<bool dummy = multiEdges, typename = std::enable_if_t<dummy>>
	auto getEdge(unsigned from, unsigned to, const edge_type& edge) const {
		auto edges = this->edges(from, to);
		while (!edges.empty()) {
			const edge_type& this_e = *edges;
			if (this_e == edge)
				return edges.begin();
			++edges;
		}

		return edges.end();
	}

	// ���Զ��ͼ��Ч
	template<bool dummy = multiEdges, typename = std::enable_if_t<dummy>>
	bool hasEdge(unsigned from, unsigned to, const edge_type& edge) const {
		return getEdge(from, to, edge) != edges(from, to).end();
	}


protected:

	constexpr static bool vertexHasOutDegree_() {
		return has_outdegree_v<underly_vertex_t>;
	}

	template<typename VERTEX_TYPE = underly_vertex_t,
		std::enable_if_t<has_outdegree_v<VERTEX_TYPE>, bool> = true>
	decltype(auto) outdegree_(unsigned v) const {
		return vertex_traits<VERTEX_TYPE>::outdegree(super_::vertexAt(v));
	}
};


/////////////////////////////////////////////////////////////////////////////////////
/// �ػ�immutable�ӿڣ���Ӧ��level = 1

template<typename GRAPH_IMPL, bool digraph, bool multiEdges, bool alwaysSorted>
class KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 1>
	: public KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 0>
{
public:
	using super_ = KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 0>;
	using typename super_::edge_type;
	using typename super_::underly_edge_t;
	using super_::edges; // ����const�汾��edges


	// �ṩһ����д�汾��edgesʵ��
	auto edges(unsigned from, unsigned to) {
		decltype(auto) r = super_::outedges(from);
		if constexpr (isDense()) {
			auto first = std::next(r.begin(), to);
			if (*first == super_::null_edge)
				return KtRange(first, 0);
			else
				return KtRange(first, 1);
		}
		else if constexpr (isAlwaysSorted()) {
			auto range = std::equal_range(r.begin(), r.end(), to, 
				kPrivate::KpEdgeCompAllInOne<underly_edge_t>{});
			return KtRange(range.first, range.second);
		}
		else { // ��Ҫ����ȫ�������ռ�<from, to>������multi-edges

			auto pred = [to](typename decltype(r)::const_deref_type e) {
					return to == edge_traits<underly_edge_t>::to(e); 
			};

			auto pos = std::find_if(r.begin(), r.end(), pred);
			if constexpr (!isMultiEdges()) {
				return KtRange(pos, pos == r.end() ? 0 : 1);
			}
			else {
				return KtCondRange<decltype(pos), decltype(pred)>{ pos, r.end(), pred };
			}
		}
	}


	// ���Զ��ͼ��Ч
	template<bool dummy = !multiEdges, typename = std::enable_if_t<dummy>>
	void setEdge(unsigned from, unsigned to, const edge_type& edge) {
		assert(super_::hasEdge(from, to));
		auto edges = this->edges(from, to);
		assert(edges.size() == 1);
		*edges = edge;
		if constexpr (!isDigraph())
			*this->edges(to, from) = edge;
	}


	// ���Զ��ͼ��Ч
	template<bool dummy = multiEdges, typename = std::enable_if_t<dummy>>
	void setEdge(unsigned from, unsigned to, const edge_type& curEdge, const edge_type& newEdge) {
		assert(hasEdge(from, to, curEdge));
		decltype(auto) iter = getEdge_(from, to, curEdge);
		*iter = newEdge;
		if constexpr (!isDigraph())
			if (from != to) {
				assert(hasEdge(to, from, curEdge));
				decltype(auto) iter = getEdge_(to, from, curEdge);
				*iter = newEdge;
			}
	}


	using super_::getEdge;

	// �������򵥱�ͼ�ṩgetEdge����ֵ�ӿڣ�����ͼ����
	// ��Ϊ����ͼʵ���ϱ����������ߣ���Ҫ����Ȩֵһ����
	template<bool dummy = !multiEdges && digraph, typename = std::enable_if_t<dummy>>
	edge_type& getEdge(unsigned from, unsigned to) {
		auto edges = this->edges(from, to);
		assert(edges.size() == 1);
		return *edges;
	}


protected:

	// ���Զ��ͼ��Ч
	template<bool dummy = multiEdges, typename = std::enable_if_t<dummy>>
	auto getEdge_(unsigned from, unsigned to, const edge_type& edge) {
		auto edges = this->edges(from, to);
		while (!edges.empty()) {
			const edge_type& this_e = *edges;
			if (this_e == edge)
				return edges.begin();
			++edges;
		}

		return edges.end();
	}
};


/////////////////////////////////////////////////////////////////////////////////////
/// �ػ�reshapable�ӿڣ���Ӧ��level = 2
//

template<typename GRAPH_IMPL, bool digraph, bool multiEdges, bool alwaysSorted>
class KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 2>
	: public KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 1>
{
public:
	using super_ = KtGraph<GRAPH_IMPL, digraph, multiEdges, alwaysSorted, 1>;
	using typename super_::edge_type;
	using underly_vertex_t = typename graph_traits<super_>::underly_vertex_t;
	using underly_edge_t = typename graph_traits<super_>::underly_edge_t;
	using edge_iter = typename graph_traits<super_>::edge_iter;
	using const_edge_iter = typename graph_traits<super_>::const_edge_iter;


	KtGraph() = default;

	KtGraph(unsigned nv) {
		reset(nv);
	}


	// addVertexֱ��ʹ�û����ʵ��
	using super_::addVertex;

	void addEdge(unsigned from, unsigned to, const edge_type& edge) {
		addEdge_(from, to, edge);
		if constexpr (!isDigraph()) {
			if (from != to)
				addEdge_<true>(to, from, edge);
		}
	}


	// ��edge��to���ԣ��ṩһ���򻯰��addEdge�ӿ�
	template<typename EDGE_TYPE, 
		std::enable_if_t<has_to_v<EDGE_TYPE>, bool> = true>
	void addEdge(unsigned from, const EDGE_TYPE& edge) {
		addEdge(from, edge_traits<EDGE_TYPE>::to(edge), edge);
	}


	// ��������edge_type�����͹��죩�����ṩһ��addEdge�ļ򻯰�
	template<typename EDGE_TYPE = edge_type, 
		std::enable_if_t<std::is_constructible_v<EDGE_TYPE, int>, bool> = true>
	void addEdge(unsigned from, unsigned to) {
		addEdge(from, to, EDGE_TYPE{ 1 });
	}


	using super_::eraseEdge; // ���������eraseEdge��KtAdjIterҪ�õ�

	// ɾ��from��to�����б�
	void eraseEdge(unsigned from, unsigned to) {
		eraseEdge_(from, to);
		if constexpr (!isDigraph()) {
			if (from != to)
				eraseEdge_<true>(to, from);
		}
	}


	// ���Զ��ͼ��Ч
	template<bool dummy = multiEdges, typename = std::enable_if_t<dummy>>
	void eraseEdge(unsigned v, unsigned w, const edge_type& val) {
		assert(hasEdge(v, w, val));

		auto pos = getEdge_(v, w, val);
		super_::eraseEdge(v, pos);
		if constexpr (super_::vertexHasOutDegree_())
			outdegree_(v)--;

		if constexpr (!isDigraph()) 
			if (v != w) {
				super_::eraseEdge<true>(w, getEdge_(w, v, val));
				if constexpr (super_::vertexHasOutDegree_())
					outdegree_(w)--;
			}
	}


	// TODO: �Ż�
	void eraseOutEdges(unsigned v) {
		if constexpr (isDense()) {
			for (unsigned u = 0; u < order(); u++)
				eraseEdgeIfExist_(v, u);
		}
		else {
			decltype(auto) r = super_::outedges(v);

			std::set<unsigned> us; // ��������flatͼ�ıߴ洢���֣��ȱ�������ߵ�to���㣬������ɾ������ͼ(to, from)��
			                       // �����ִ��ɾ�����������ܻ�Ӱ������r�еĵ�����
			                       // ���Ƕ�ߣ�ѡ��set����
			if constexpr (!isDigraph()) 
				for (auto& e : r)
					us.insert(to_(e));

			if constexpr (super_::vertexHasOutDegree_())
				outdegree_(v) -= r.size();
			super_::eraseEdges(v, r.begin(), r.end());

			if constexpr (!isDigraph())
				for(auto u : us)
					if(u != v)
				        eraseEdge_<true>(u, v);
		}
	}


	void eraseInEdges(unsigned v) {
		if constexpr (!isDigraph()) {
			eraseOutEdges(v);
		}
		else {
			for (unsigned u = 0; u < order(); u++) 
				eraseEdgeIfExist_(u, v);
		}
	}


	// ɾ���붥��v��ӵ����б�
	void eraseEdges(unsigned v) {
		eraseOutEdges(v);
		if constexpr (isDigraph())
			eraseInEdges(v);
	}


	void eraseVertex(unsigned v) {
		eraseEdges(v);
		super_::eraseVertex(v);

		// �������ߵ�toֵ
		// TODO: ����alwaysSorted�Ż�
		if constexpr (!isDense()) {
			for (unsigned i = 0; i < order(); i++) {
				decltype(auto) r = super_::outedges(i);
				for (auto& e : r) {
					assert(to_(e) != v);
					if (to_(e) > v) to_(e)--;
				}
			}
		}
	}


private:

	// @dummy: ��Ϊtrue�����ʾ���ӵ�������ͼ�ߵĻر�
	template<bool dummy = false>
	void addEdge_(unsigned from, unsigned to, const edge_type& edge) {
		decltype(auto) r = super_::outedges(from);
		if constexpr (isDense()) {
			super_::addEdge<dummy>(from, std::next(r.begin(), to), edge);
		}
		else if constexpr (!isAlwaysSorted()) {
			static_assert(has_to_v<underly_edge_t>, "edge missing property of 'to'");

			auto iter = super_::addEdge<dummy>(from, r.end(), edge); // �����������ӱߵ�β��
			to_(*iter) = to;
		}
		else {
			auto pos = std::lower_bound(r.begin(), r.end(), to, 
				kPrivate::KpEdgeCompAllInOne<underly_edge_t>{});
			auto iter = super_::addEdge<dummy>(from, pos, edge);
			to_(*iter) = to;
		}

		if constexpr (super_::vertexHasOutDegree_())
			outdegree_(from)++;
	}


	template<bool dummy = false>
	void eraseEdge_(unsigned from, unsigned to) {
		auto edges = this->edges(from, to);
		assert(edges.size() > 0);
			
		if constexpr (!isMultiEdges()) {
			assert(edges.size() == 1);
			super_::eraseEdge<dummy>(from, edges.begin());

			if constexpr (super_::vertexHasOutDegree_())
				outdegree_(from)--;
		}
		else if constexpr (isAlwaysSorted()) {
			super_::eraseEdges<dummy>(from, edges.begin(), edges.end());

			if constexpr (super_::vertexHasOutDegree_())
				outdegree_(from) -= edges.size();
		}
		else {// δ����Ķ�ߣ����μ��
			do {
				edges.begin() = super_::eraseEdge<dummy>(from, edges.begin());	
				edges.end() = super_::outedges(from).end();
				if constexpr (super_::vertexHasOutDegree_())
					outdegree_(from)--;

				edges.meetCond_();
			} while (!edges.empty());
		}
	}


	void eraseEdgeIfExist_(unsigned from, unsigned to) {
		if (!edges(from, to).empty()) {
			eraseEdge_(from, to);
			if constexpr (!isDigraph())
				if(from != to )
				    eraseEdge_<true>(to, from);
		}
	}


	template<typename EDGE_TYPE,
		std::enable_if_t<has_to_v<EDGE_TYPE>
		    && !std::is_const_v<EDGE_TYPE>, bool> = true>
	static decltype(auto) to_(const EDGE_TYPE& e) {
		return edge_traits<EDGE_TYPE>::to(e);
	}

	template<typename EDGE_TYPE,
		std::enable_if_t<has_to_v<EDGE_TYPE>, bool> = true>
	static decltype(auto) to_(EDGE_TYPE& e) {
		return edge_traits<EDGE_TYPE>::to(e);
	}


	template<typename VERTEX_TYPE = underly_vertex_t,
		std::enable_if_t<has_outdegree_v<VERTEX_TYPE>, bool> = true>
	decltype(auto) outdegree_(unsigned v) {
		return vertex_traits<VERTEX_TYPE>::outdegree(super_::vertexAt(v));
	}
};