#pragma once
#include "KtGraphBase.h"
#include "../base/KtMatrix.h"


// 稠密图特有实现
// 此处主要实现邻接顶点迭代器
template<typename T, bool direction = false> 
class KtGraphDense : public KtGraphBase<KtMatrix<T>, direction, false>
{
public:
    using super_ = KtGraphBase<KtMatrix<T>, direction>;
    using adj_matrix_type = KtMatrix<T>;
    using typename super_::edge_type;
    using typename super_::const_edge_ref;
    using typename super_::vertex_index_t;
    using super_::KtGraphBase; // 导入基类的构造函数
    using super_::adjMat_;
    using super_::null_;
    using super_::order;


    // 重载个虚函数
    virtual unsigned outdegree(unsigned v) const override {
        return order() - adjMat_.row(v).count(null_);
    }


private:

    // 邻接顶点迭代器的基类
    template<typename GRAPH, typename RANGE>
    class adj_vertex_iter_base_
    {
    public:
        using deref_type = decltype(*std::declval<RANGE>());
        using const_deref_type = const deref_type;

        adj_vertex_iter_base_(GRAPH& g) : graph_(g), u_(GRAPH::null_vertex) {
            v_ = GRAPH::null_vertex, null_ = g.nullEdge();
        }

        adj_vertex_iter_base_(GRAPH& g, const RANGE& r, const vertex_index_t idx) 
            : graph_(g), range_(r), u_(idx) {
            v_ = 0, null_ = g.nullEdge();
            while(!isEnd() && *range_ == null_) // skip null element
                ++range_, ++v_;
        }

        adj_vertex_iter_base_(const adj_vertex_iter_base_& iter) = default;

        
        const vertex_index_t source() const { return u_; }
        const vertex_index_t operator*() const { return v_; }


        void operator++() {
            assert(!isEnd());
            ++range_, ++v_;
             while(!isEnd() && *range_ == null_) { // skip null element
                ++range_, ++v_;
            }           
        }

        bool isEnd() const { return range_.empty(); }


        // get the value of edge (u_, v_)
        const_deref_type edge() const { return *range_; }

        // reset the value of edge (u_, v_)
        void reedge(const_edge_ref val) {
            assert(val != null_);
            *range_ = val; 

            if (!graph_.isDigraph() && v_ != u_)
                graph_.adjMatrix(v_, u_) = val;
        }

        // erase the edge (u_, v_)
        void erase() {
            *range_ = null_;
            ++(*this); // skip the erased element

            if (!graph_.isDigraph() && v_ != u_)
                graph_.adjMatrix()(v_, u_) = null_;
        }

    private:
        GRAPH& graph_;
        RANGE range_;
        const vertex_index_t u_; // range_对应的行号/列号
        vertex_index_t v_; 
        edge_type null_;
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

        adj_vertex_iter_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_base(graph_type& g, const vertex_index_t v) :
            super_(g, g.adjMatrix().row(v), v) {}
    };

    template<bool bConst>
    class adj_vertex_iter_r_base : public adj_vertex_iter_base_<graph_type_<bConst>, range_type_<bConst, false>>
    {
    public:
        using graph_type = graph_type_<bConst>;
        using range_type = range_type_<bConst, false>;

        using super_ = adj_vertex_iter_base_<graph_type, range_type>;

        adj_vertex_iter_r_base(graph_type& g) : super_(g) {}

        adj_vertex_iter_r_base(graph_type& g, const vertex_index_t v) :
            super_(g, g.adjMatrix().col(v), v) {}
    };

public:

    using adj_vertex_iter = adj_vertex_iter_base<false>;
    using const_adj_vertex_iter = adj_vertex_iter_base<true>;
    using adj_vertex_iter_r = adj_vertex_iter_r_base<false>;
    using const_adj_vertex_iter_r = adj_vertex_iter_r_base<true>;
};

