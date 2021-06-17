#pragma once
#include <vector>
#include <cassert>


// 基于邻接矩阵的图布局
// MAT_TYPE为邻接矩阵类型，至少要有以下3项成员：
//     1.类型声明：value_type
//     2.构造函数：MAT_TYPE(unsigned numRow, unsigned numCol, value_type initValue)
//     3.符号重载：value_type& operaotr(unsigned idxRow, unsigned idxCol)
// direction为真代表有向图，否则代表无向图。
template<typename MAT_TYPE, bool direction = false> 
class KtGraphBase
{
public:
    typedef typename MAT_TYPE::value_type value_type;

    KtGraphBase() : V_(0), E_(0), null_(0) {} 
    KtGraphBase(const KtGraphBase&) = default;
    KtGraphBase(KtGraphBase&&) = default;
    KtGraphBase& operator=(const KtGraphBase&) = default;
    KtGraphBase& operator=(KtGraphBase&&) = default;

    explicit KtGraphBase(unsigned numVertex, value_type nullValue = value_type{0}) : 
        adjMat_(numVertex, numVertex, nullValue),
        V_(numVertex),
        E_(0),
        null_(nullValue) {}

    constexpr static bool isDigraph() { return direction; }

    // 重置图为numVertex顶点，并清空所有边
    void reset(unsigned numVertex, value_type nullValue = value_type(0)) {
        V_ = numVertex, E_ = 0;
        null_ = nullValue;
        adjMat_.reset(numVertex, numVertex, null_);
    }

    // 返回图的阶，即顶点数目
    unsigned order() const { return V_; }

    // 返回图的边数
    unsigned size() const { return E_; }

    // 空图
    bool isEmpty() const  { return order() == 0; }

    // 平凡图
    bool isTrivial() const { return order() == 1; }

    // 零图
    bool isNull() const { return size() == 0; }

    const value_type& getEdge(unsigned v1, unsigned v2) const {
        return adjMat_(v1, v2);
    }

    value_type getEdge(unsigned v1, unsigned v2) {
        return adjMat_(v1, v2);
    }

    bool hasEdge(unsigned v1, unsigned v2) const {
        return getEdge(v1, v2) != null_;
    }


    void addEdge(unsigned v1, unsigned v2, const value_type& val = value_type(1)) {
        assert(val != null_);
        if(!hasEdge(v1, v2)) 
            ++E_;
        adjMat_(v1, v2) = val;
        if(!direction) adjMat_(v2, v1) = val;
     }
     

    void setEdge(unsigned v1, unsigned v2, const value_type& val) {
        assert(val != null_ && hasEdge(v1, v2));
        adjMat_(v1, v2) = val;
        if(!direction) adjMat_(v2, v1) = val;
    }


    void eraseEdge(unsigned v1, unsigned v2) {
        if(hasEdge(v1, v2)) {
            adjMat_(v1, v2) = null_;
            if(!direction) adjMat_(v2, v1) = null_;
            --E_;
        }
    }


	// 出度
	virtual unsigned outdegree(unsigned v) const {
		unsigned d(0);
		for (unsigned i = 0; i < order(); i++)
			if (hasEdge(v, i)) ++d;
		return d;
	}


	// 入度
	unsigned indegree(unsigned v) const {
		if (!direction) {
			return outdegree(v);
		}
		else {
			unsigned d(0);
			for (unsigned i = 0; i < order(); i++)
				if (hasEdge(i, v)) ++d;
			return d;
		}
	}

	// 度
	unsigned degree(unsigned v) const {
		auto d = outdegree(v);
		if (direction) d += indegree(v);
		return d;
	}


	// 删除顶点v及其邻边
	void eraseVertex(unsigned v) {
		auto d = degree(v);
		if (direction && hasEdge(v, v)) --d;
		adjMat_.eraseRow(v);
		adjMat_.eraseCol(v);
		--V_;
		E_ -= d;
	}


protected:
    MAT_TYPE adjMat_; // 邻接矩阵
    value_type null_; // 矩阵元素等于null_时，代表无连接
    unsigned V_; // 顶点数量
    unsigned E_; // 边数量
};
