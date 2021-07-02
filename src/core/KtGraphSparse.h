#pragma once
#include "KtGraphBase.h"
#include "../base/KtSparseMatrix.h"


template<typename T, bool direction = false> 
class KtGraphSparse : public KtGraphBase<KtSparseMatrix<T>, direction>
{
public:
    using super_ = KtGraphBase<KtSparseMatrix<T>, direction>;

    using super_::value_type;
    using super_::adjMat_;
    using super_::hasEdge;
    using super_::E_;

    // ¼Ì³Ð¹¹Ôìº¯Êý
    using super_::KtGraphBase;


	// É¾³ý±ßv-w
    void eraseEdge(unsigned v, unsigned w) {
        if(hasEdge(v, w)) {
            adjMat_.setDefault(v, w);
            if(!direction) adjMat_.setDefault(w, v);
            --E_;
        }
    }


    class adj_vertex_iter
    {
    public:
        adj_vertex_iter(const adj_vertex_iter&) = default;
        adj_vertex_iter(adj_vertex_iter&&) = default;
        adj_vertex_iter& operator=(const adj_vertex_iter&) = default;
        adj_vertex_iter& operator=(adj_vertex_iter&&) = default; 

		// dummy empty iter
		adj_vertex_iter(const KtGraphSparse& g) : 
			iter(g.adjMat_.rowEnd(0)), end(g.adjMat_.rowEnd(0)) {}

		// iter of v
        adj_vertex_iter(const KtGraphSparse& g, unsigned v) : 
            iter(g.adjMat_.rowBegin(v)), end(g.adjMat_.rowEnd(v)) {}

    public:
        unsigned operator*() const { return iter->first; } 

		auto value() const { return iter->second; }
		auto value() { return iter->second; }

        void operator++() { ++iter; }

        bool isEnd() const { return iter == end; }

    private:
        typename KtSparseMatrix<T>::const_element_iterator iter, end;
    };


	virtual unsigned outdegree(unsigned v) const override {
		return adjMat_.getSizeOfRow(v);
	}
};

