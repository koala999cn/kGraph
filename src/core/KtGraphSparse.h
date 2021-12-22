#pragma once
#include "KtGraphBase.h"
#include "../base/KtSparseMatrix.h"


template<typename T, bool direction = false, bool parallel = false>
class KtGraphSparse : public KtGraphBase<KtSparseMatrix<T, parallel>, direction, parallel>
{
public:
    using super_ = KtGraphBase<KtSparseMatrix<T, parallel>, direction, parallel>;
    using adj_matrix_type = KtSparseMatrix<T, parallel>;
    using typename super_::vertex_index_t;
    using typename super_::edge_type;
    using typename super_::const_edge_ref;
    using super_::adjMat_;
    using super_::hasEdge;
    using super_::setEdge;
    using super_::isDigraph;
    using super_::E_;
    using super_::null_;


    // 继承构造函数
    using super_::KtGraphBase;


	void reserve(unsigned numVerts, unsigned numEdges) {
		adjMat_.reserveRows(numVerts);
	}

	void reserveEdges(vertex_index_t v, unsigned numEdges) {
		adjMat_.reserveCols(v, numEdges);
	}


    // 重新实现
    void eraseEdge(unsigned v, unsigned w) {
        if(hasEdge(v, w)) {
            adjMat_.setDefault(v, w);
            if(!direction && v != w) adjMat_.setDefault(w, v);
            --E_;
        }
    }


    // 重载
    unsigned outdegree(unsigned v) const final { // 已是最优实现，使用final标记
        return adjMat_.nonDefaultsOfRow(v);
    }


    /// 新增对平行边的支持函数

    bool hasEdge(vertex_index_t v1, vertex_index_t v2, const_edge_ref val) const {
        return adjMat_(v1, v2, val) != null_;
    }

    void setEdge(vertex_index_t v1, vertex_index_t v2, const_edge_ref curVal, const_edge_ref newVal) {
        assert(curVal != null_ && newVal != null_); assert(hasEdge(v1, v2, curVal));
        adjMat_(v1, v2, curVal) = newVal;
        if (!isDigraph()) adjMat_(v2, v1, curVal) = newVal;
    }

    void eraseEdge(unsigned v, unsigned w, const_edge_ref val) {
        if (hasEdge(v, w)) {
            adjMat_.setDefault(v, w, val);
            if (!direction) adjMat_.setDefault(w, v, val);
            --E_;
        }
    }


private:

    // 邻接顶点迭代器的基类
    template<typename GRAPH, typename RANGE>
    class adj_vertex_iter_base_
    {
    public:
        using deref_type = decltype((*std::declval<RANGE>()).second);
        using const_deref_type = decltype((*std::declval<std::add_const_t<RANGE>>()).second);

        adj_vertex_iter_base_(GRAPH& g) : graph_(g), u_(GRAPH::null_vertex) {}

        adj_vertex_iter_base_(GRAPH& g, const RANGE& r, const vertex_index_t idx) 
            : graph_(g), range_(r), u_(idx) {}

        const vertex_index_t other() const { return u_; }
        const vertex_index_t operator*() const { return (*range_).first; }

        void operator++() { ++range_; }

        bool isEnd() const { return range_.empty(); }


        // get the value of current edge
        const_deref_type edge() const { return (*range_).second; }

		virtual vertex_index_t from() const = 0;
		virtual vertex_index_t to() const = 0;


		// change the value of current edge      
		void reedge(const_edge_ref val) {
			assert(val != graph_.nullEdge());

			typename RANGE::deref_type e = *range_;

			if (!graph_.isDigraph() && **this != other()) {
				if (parallel)
					graph_.adjMatrix()(to(), from(), e.second) = val;
				else
					graph_.adjMatrix()(to(), from()) = val;
			}

			e.second = val;
		}

		// erase the current edge
		void erase() {
			assert(false); // 由子类实现
		}

    protected:
        GRAPH& graph_;
        RANGE range_;
        const vertex_index_t u_; // range_对应的行号/列号
    };

    template<bool bConst>
    using graph_type_ = std::conditional_t<bConst, const super_, super_>;

    template<bool bConst>
    using row_range_type_ = std::conditional_t<bConst, typename adj_matrix_type::const_row_range,
        typename adj_matrix_type::row_range>;

    template<bool bConst>
    using col_range_type_ = std::conditional_t<bConst, typename adj_matrix_type::const_col_range,
        typename adj_matrix_type::col_range>;


    template<bool bConst, bool bRowRange>
    using range_type_ = std::conditional_t<bRowRange, row_range_type_<bConst>, col_range_type_<bConst>>;


    template<bool bConst>
    class adj_vertex_iter_base : public adj_vertex_iter_base_<graph_type_<bConst>, range_type_<bConst, true>>
    {
    public:
        using graph_type = graph_type_<bConst>;
        using range_type = range_type_<bConst, true>;

        using super_ = adj_vertex_iter_base_<graph_type, range_type>;
        using super_::graph_;
        using super_::range_;
        using super_::other;


        adj_vertex_iter_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_base(graph_type& g, vertex_index_t v) :
            super_(g, g.adjMatrix().row(v), v) {}


		virtual vertex_index_t from() const final {
			return other();
		}

		virtual vertex_index_t to() const final {
			return **this;
		}

		void erase() {
			auto& e = *range_;

			if (!graph_.isDigraph() && **this != other()) {
				if (parallel)
					graph_.adjMatrix().setDefault(to(), from(), e.second);
				else
					graph_.adjMatrix().setDefault(to(), from());
			}

			assert(range_.end() == graph_.adjMatrix().row(from()).end()); // TODO

			range_.begin() = graph_.adjMatrix().erase(from(), range_.begin());
			range_.end() = graph_.adjMatrix().row(from()).end();

			static_assert(std::is_convertible<graph_type, typename KtGraphSparse::super_>::value, "illegal type of GRAPH for KtGraphSparse::adj_vertex_iter_base_.");
			KtGraphSparse* g = dynamic_cast<KtGraphSparse*>(&graph_);
			--g->E_;
		}

    };

    template<bool bConst>
    class adj_vertex_iter_r_base : public adj_vertex_iter_base_<graph_type_<bConst>, range_type_<bConst, false>>
    {
    public:
        using graph_type = graph_type_<bConst>;
        using range_type = range_type_<bConst, false>;

        using super_ = adj_vertex_iter_base_<graph_type, range_type>;
        using super_::graph_;
        using super_::range_;
        using super_::other;


        adj_vertex_iter_r_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_r_base(graph_type& g, vertex_index_t v) :
            super_(g, g.adjMatrix().col(v), v) {}


		virtual vertex_index_t from() const final {
			return **this;
		}

		virtual vertex_index_t to() const final {
			return other();
		}


        void erase() {
			
			auto& mat = graph_.adjMatrix();
			assert(range_.end() == mat.colEnd(to()));

			bool updateEnd = (from() == mat.rows() - 1); // 若删除最后一行元素，则需要同步更新range_.end()
			typename range_type::deref_type e = *range_;

			if (!graph_.isDigraph() && **this != other()) {
				if (parallel)
					mat.setDefault(to(), from(), e.second);
				else
					mat.setDefault(to(), from());

				if (to() == mat.rows() - 1)
					updateEnd = true;
			}

			
			range_.begin().erase();
			if(updateEnd)
				range_.end() = mat.colEnd(to());

			static_assert(std::is_convertible<graph_type, typename KtGraphSparse::super_>::value, "illegal type of GRAPH for KtGraphSparse::adj_vertex_iter_r_base.");
			KtGraphSparse* g = dynamic_cast<KtGraphSparse*>(&graph_);
			--g->E_;
        }
    };

public:

    using adj_vertex_iter = adj_vertex_iter_base<false>;
    using const_adj_vertex_iter = adj_vertex_iter_base<true>;
    using adj_vertex_iter_r = adj_vertex_iter_r_base<false>;
    using const_adj_vertex_iter_r = adj_vertex_iter_r_base<true>;
};

