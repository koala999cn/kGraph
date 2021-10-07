#pragma once
#include <map>
#include <memory>
#include <assert.h>


/// 嵌套子图
template<typename BASE_GRAPH>
class KtSubGraph : public BASE_GRAPH
{
public:
    using vertex_index_t = typename BASE_GRAPH::vertex_index_t;
    using SUB_CONTAINER = std::map<vertex_index_t, std::shared_ptr<KtSubGraph<BASE_GRAPH>>>;
    using BASE_GRAPH::BASE_GRAPH;


    /// 展开所有的子图/嵌套子图
    void expandTo(BASE_GRAPH& g) const;


    /// 是否包含子图
    bool hasSub() const {
        return !subs_.empty();
    }


    /// 顶点v是否为子图
    bool isSub(vertex_index_t v) const {
        return subs_.find(v) != subs_.end();
    }


    /// 设置/取消顶点v为子图顶点
    void attachSub(vertex_index_t v, std::shared_ptr<KtSubGraph>& sub) {
        subs_.insert_or_assign(v, sub);
    }

    void detachSub(vertex_index_t v) {
        subs_.erase(v);
    }


    /// 子图迭代
    auto subBegin() { return subs_.begin(); }
    auto subEnd() { return subs_.end(); }
    auto subBegin() const { return subs_.cbegin(); }
    auto subEnd() const { return subs_.cend(); }

private:

    /* 计算THIS展开后的vertex和edge数量 */
    void calcExpandedSize_(unsigned& V, unsigned& E) const;


    // 将this作为g的第subNode顶点的子图进行展开，顶点偏移为offset。
    // 若subNode == -1，表示当前图为顶层图。
    void expandTo_(BASE_GRAPH& g, vertex_index_t subNode, vertex_index_t& offset) const;


    // 将子图中的顶点v转换到父图所对应的顶点序号
    //   @start: 子图的源点
    //   @offset: 子图从父图的offset顶点开始展开
    static vertex_index_t subToSup(vertex_index_t v, vertex_index_t start, vertex_index_t offset) {
        return v < start ? v + offset : v + offset - 1; /// 兼容start == -1的情况
    }


private:
    SUB_CONTAINER subs_;
};


template<typename BASE_GRAPH>
void KtSubGraph<BASE_GRAPH>::calcExpandedSize_(unsigned& V, unsigned& E) const
{
    V += BASE_GRAPH::order();
    E += BASE_GRAPH::size();

    for (auto& i : subs_)
        i.second->calcExpandedSize_(V, E);

    V -= subs_.size(); /// sub-graph顶点可重复利用
}


template<typename BASE_GRAPH>
void KtSubGraph<BASE_GRAPH>::expandTo(BASE_GRAPH& g) const
{
    unsigned V(0), E(0);
    calcExpandedSize_(V, E);
    g.reserve(V, E);
    g.resize(V); 

    vertex_index_t offset(0);
    expandTo_(g, -1, offset);

    assert(offset == V && g.size() == E);
}


template<typename BASE_GRAPH>
void KtSubGraph<BASE_GRAPH>::expandTo_(BASE_GRAPH& g, vertex_index_t subNode, vertex_index_t& offset) const
{
    /// first step: expand this->g_ to g 
    vertex_index_t start(-1); // THIS的源点
    vertex_index_t end(-1); // THIS的汇点

    // 1. copy vertex objects & adjust g'out-edges of sub-graph vertex [subNode]
    if (subNode != -1) { /// THIS子图对应于g的subNode顶点
        auto s = BASE_GRAPH::sources();
        assert(s.size() == 1);
        start = s[0];

        // TODO: 支持非顶点对象图
        g.getVertex(subNode) = BASE_GRAPH::getVertex(start); // subNode顶点复用为子图的源点

        auto e = BASE_GRAPH::sinks(); 
        assert(e.size() == 1);
        end = subToSup(e[0], start, offset);

        // 将图g中subNode顶点的所有出边，起始点由subNode调整为end
        auto iter = g.adjIter(subNode);
        std::vector<vertex_index_t> ws; ws.reserve(g.outdegree(subNode));
        for (; !iter.isEnd(); ++iter) {
            g.addEdge(end, *iter, iter.value());
            ws.push_back(*iter);
        }
        for (auto w : ws)
            g.eraseEdge(subNode, w);
    }

    for (auto i = 0u; i < BASE_GRAPH::order(); i++)
        if (i != start)
            g.getVertex(offset + i) = BASE_GRAPH::getVertex(i);

    // 2. copy edges
    typename BASE_GRAPH::edge_iter iter(*this, 0);
    for (; !iter.isEnd(); ++iter) {
        vertex_index_t v = iter.from(), w = *iter;
        assert(w != start); /// 源点不可能有入边
        g.addEdge(v == start ? subNode : subToSup(v, start, offset), subToSup(w, start, offset), iter.value());
    }

    // 3. update [offset]
    offset += (subNode != -1) ? BASE_GRAPH::order() - 1 : BASE_GRAPH::order();


    /// second step: expand all sub-graphs
    for (const auto& i : subs_) {
        assert(i.first != start && i.first != end);
        i.second->expandTo_(g, subToSup(i.first, start, offset), offset);
    }
}
