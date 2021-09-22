#pragma once
#include "KtGraphBase.h"
#include "../base/KtMatrix.h"


template<typename T, bool direction = false> 
class KtGraphDense : public KtGraphBase<KtMatrix<T>, direction, false>
{
public:
    using super_ = KtGraphBase<KtMatrix<T>, direction>;
    using typename super_::value_type;

    using super_::KtGraphBase; // 导入基类的构造函数
    using super_::adjMat_;
    using super_::order;
    using super_::null_;

    class adj_vertex_iter
    {
    public:
        adj_vertex_iter(const adj_vertex_iter&) = default;
        adj_vertex_iter(adj_vertex_iter&&) = default;
        adj_vertex_iter& operator=(const adj_vertex_iter&) = default;
        adj_vertex_iter& operator=(adj_vertex_iter&&) = default;
        
        // dummy empty iter
        adj_vertex_iter(const KtGraphDense& g) : v_(-1), null_(g.null_),
            iter_(g.adjMat_.rowEnd(0)), end_(g.adjMat_.rowEnd(0)) {}

        // iter of v
        adj_vertex_iter(const KtGraphDense& g, unsigned v) : v_(0), null_(g.null_),
            iter_(g.adjMat_.rowBegin(v)), end_(g.adjMat_.rowEnd(v)) { 
            while(!isEnd() && *iter_ == g.null_) { // skip null element
                ++iter_, ++v_;
            }
        }


        unsigned operator*() const { return v_; } 

        auto value() const { return *iter_; }

        void operator++() {
            assert(!isEnd());
            ++iter_, ++v_;
             while(!isEnd() && *iter_ == null_) { // skip null element
                ++iter_, ++v_;
            }           
        }

        bool isEnd() const { return iter_ == end_; }

    private:
        typename KtMatrix<T>::const_element_iterator iter_, end_;
        unsigned v_;
        value_type null_;
    };


    virtual unsigned outdegree(unsigned v) const override { 
        auto begin = adjMat_.rowBegin(v), end = adjMat_.rowEnd(v);
        return order() - std::count(begin, end, null_);
    }

};

