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


    // 重新实现
    void eraseEdge(unsigned v, unsigned w) {
        if(hasEdge(v, w)) {
            adjMat_.setDefault(v, w);
            if(!direction && v != w) adjMat_.setDefault(w, v);
            --E_;
        }
    }


    // 重载
    virtual unsigned outdegree(unsigned v) const final { // 已是最优实现，使用final标记
        return adjMat_.nonDefaultsOfRow(v);
    }


    // 新增对平行边的支持函数

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
        using deref_type = decltype(std::declval<RANGE>()->second);
        using const_deref_type = const deref_type;

        adj_vertex_iter_base_(GRAPH& g) : graph_(g), u_(GRAPH::null_vertex) {}

        adj_vertex_iter_base_(GRAPH& g, const RANGE& r, const vertex_index_t idx) 
            : graph_(g), range_(r), u_(idx) {}

        const vertex_index_t source() const { return u_; }
        const vertex_index_t operator*() const { return range_->first; }

        void operator++() { ++range_; }

        bool isEnd() const { return range_.empty(); }


        // get the value of edge (u_, v_)
        const_deref_type edge() const { return range_->second; }


        // change the value of edge (u_, v_)
        void reedge(const_edge_ref val) {
            assert(val != null_);

            if (!graph_.isDigraph() && range_->first != u_) {
                if (parallel)
                    graph_.adjMatrix(range_->first, u_, range_->second) = val;
                else
                    graph_.adjMatrix(range_->first, u_) = val;
            }

            range_->second = val;
        }


        // erase the edge (u_, v_)
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
        using super_::u_;


        adj_vertex_iter_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_base(graph_type& g, vertex_index_t v) :
            super_(g, g.adjMatrix().row(v), v) {}

        void erase() {
            if (!graph_.isDigraph() && range_->first != u_) {
                if (parallel)
                    graph_.adjMatrix.setDefault(range_->first, u_, range_->second);
                else
                    graph_.adjMatrix.setDefault(range_->first, u_);
            }

            range_.begin() = graph_.adjMatrix.erase(u_, range_.begin());
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
        using super_::u_;


        adj_vertex_iter_r_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_r_base(graph_type& g, vertex_index_t v) :
            super_(g, g.adjMatrix().col(v), v) {}

        void erase() {
            if (!graph_.isDigraph() && range_->first != u_) {
                if (parallel)
                    graph_.adjMatrix.setDefault(range_->first, u_, range_->second);
                else
                    graph_.adjMatrix.setDefault(range_->first, u_);
            }

            range_.erase();
        }
    };

public:

    using adj_vertex_iter = adj_vertex_iter_base<false>;
    using const_adj_vertex_iter = adj_vertex_iter_base<true>;
    using adj_vertex_iter_r = adj_vertex_iter_r_base<false>;
    using const_adj_vertex_iter_r = adj_vertex_iter_r_base<true>;
};

