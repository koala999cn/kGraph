#pragma once
#include <vector>
#include <assert.h>


/// 基于邻接矩阵的图布局
//  -- Container为邻接矩阵类型.
//  -- direction为真代表有向图，否则代表无向图。
//  -- parallel为真代表允许平行边，即两点之间可有多条边。
template<typename ADJ_MATRIX, bool direction = false, bool parallel = false>
class KtGraphBase
{
public:
    using adj_matrix_type = ADJ_MATRIX;
    using edge_type = typename adj_matrix_type::value_type;
    using edge_ref = typename adj_matrix_type::reference;
    using const_edge_ref = typename adj_matrix_type::const_reference;
    using vertex_index_t = unsigned;
    constexpr static vertex_index_t null_vertex = -1;


    KtGraphBase() : E_(0), null_{} { } // null默认为零初始化值

    KtGraphBase(const KtGraphBase&) = default;
    KtGraphBase(KtGraphBase&&) = default;
    KtGraphBase& operator=(const KtGraphBase&) = default;
    KtGraphBase& operator=(KtGraphBase&&) = default;

    explicit KtGraphBase(unsigned numVertex, const_edge_ref nullEdge = edge_type{}) :
        adjMat_(numVertex, numVertex, nullEdge),
        E_(0),
        null_(nullEdge) { }

    constexpr static bool isDigraph() { return direction; }

    // 重置图为numVertex顶点，并清空所有边
    void reset(unsigned numVertex, const_edge_ref nullEdge = edge_type{}) {
        E_ = 0;
        null_ = nullEdge;
        adjMat_.resize(numVertex, numVertex, null_);
    }

	// 预留numVerts个顶点和numEdges条边的存储. 实现相关，缺省无动作
    void reserve(unsigned numVerts, unsigned numEdges) { }

	// 对顶点v预留numEdges条边的存储. 实现相关，缺省无动作
	void reserveEdges(vertex_index_t v, unsigned numEdges) { }

    // 返回图的阶，即顶点数目
    unsigned order() const { return adjMat_.rows(); }

    // 返回图的边数
    unsigned size() const { return E_; }

    // 空图
    bool isEmpty() const  { return order() == 0; }

    // 平凡图
    bool isTrivial() const { return order() == 1; }

    // 零图
    bool isNull() const { return size() == 0; }

    const_edge_ref getEdge(vertex_index_t v1, vertex_index_t v2) const {
        return adjMat_(v1, v2);
    }

    edge_ref getEdge(vertex_index_t v1, vertex_index_t v2) {
        return adjMat_(v1, v2);
    }

    bool hasEdge(vertex_index_t v1, vertex_index_t v2) const {
        return getEdge(v1, v2) != null_;
    }


    void addEdge(vertex_index_t v1, vertex_index_t v2, const_edge_ref val) {
        assert(val != null_);
        assert(parallel || !hasEdge(v1, v2));
        ++E_;
        adjMat_.insert(v1, v2, val);
        if (!isDigraph()) adjMat_.insert(v2, v1, val);
    }


    // 若允许（edge_type可整型构造），则提供一个addEdge的简化版
    template<typename T = edge_type>
    typename std::enable_if_t<std::is_constructible<T, int>::value>
    addEdge(vertex_index_t v1, vertex_index_t v2) {
		assert(null_ != T{ 1 });
        addEdge(v1, v2, T{ 1 });
    }
   

    void setEdge(vertex_index_t v1, vertex_index_t v2, const_edge_ref val) {
        assert(val != null_); assert(hasEdge(v1, v2));
        adjMat_(v1, v2) = val;
        if(!isDigraph()) adjMat_(v2, v1) = val;
    }


    // 删除边(v1, v2)
    void eraseEdge(vertex_index_t v1, vertex_index_t v2) {
        if(hasEdge(v1, v2)) {
            adjMat_(v1, v2) = null_;
            if(!isDigraph()) adjMat_(v2, v1) = null_;
            --E_;
        }
    }


    // 删除顶点v的所有入边
    void eraseInEdges(vertex_index_t v) {
        E_ -= indegree(v);
        adjMat_.assignCol(v, null_);
        if (!isDigraph()) adjMat_.assignRow(v, null_);
    }


    // 删除顶点v的所有出边
    void eraseOutEdges(vertex_index_t v) {
        E_ -= outdegree(v);
        adjMat_.assignRow(v, null_);
        if (!isDigraph()) adjMat_.assignCol(v, null_);
    }


    // 删除与顶点v相接的所有边
    void eraseEdges(vertex_index_t v) {
        adjMat_.eraseRow(v);
        adjMat_.eraseCol(v);
    }


    // 出度
    virtual unsigned outdegree(vertex_index_t v) const {
        unsigned d(0);
        for (unsigned i = 0; i < order(); i++)
            if (hasEdge(v, i)) ++d;
        return d;
    }


    // 入度
    virtual unsigned indegree(vertex_index_t v) const {
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

    // 度. 注：对于有向图的自环，度为2，1个入度+1个出度
    unsigned degree(vertex_index_t v) const {
        auto d = outdegree(v);
        if (isDigraph()) d += indegree(v);
        return d;
    }


    // 删除顶点v及其邻边
    void eraseVertex(vertex_index_t v) {
        auto d = degree(v); // 与v相邻的边数
        if (isDigraph() && hasEdge(v, v)) --d; // 修正有向自环的边数
        adjMat_.eraseRow(v);
        adjMat_.eraseCol(v);
        E_ -= d;
    }


    // 增加1个孤立点
    vertex_index_t addVertex() {
        adjMat_.appendRow(null_);
        adjMat_.appendCol(null_);
        return adjMat_.rows() - 1;
    }


    const edge_type& nullEdge() const { return null_; }


	// TODO: 尽量删除以下2个函数
    adj_matrix_type& adjMatrix() { return adjMat_; }
    const adj_matrix_type& adjMatrix() const { return adjMat_; }


protected:
    adj_matrix_type adjMat_; // 邻接矩阵
    edge_type null_; // 矩阵元素等于null_时，代表无连接
    unsigned E_; // 边数量
};
