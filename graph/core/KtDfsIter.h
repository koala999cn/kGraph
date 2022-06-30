#pragma once
#include <vector>
#include <map>
#include <assert.h>


// 统一有向图与无向图的深度优先遍历模板框架，通过模板参数可支持多种遍历范式。
// 模板参数：
//    -- stopAtPopping，若为true，则每个顶点出栈时迭代器暂停，用户有机会处理出栈顶点.
//    -- fullGraph，参见KtBfsIter.
//    -- modeEdge，参见KtBfsIter.
// 当modeEdge为true， stopAtPopping为false时，则实质为边迭代器，起始顶点被跳过，已保证起始状态即为第一条边
// 当modeEdge为true， stopAtPopping为true时，初始状态仍返回起始顶点，此时from()返回-1.
template<typename GRAPH, bool fullGraph = false, bool modeEdge = false, bool stopAtPopping = false>
class KtDfsIter
{
public:
    using graph_type = GRAPH;
    using vertex_index_t = typename graph_type::vertex_index_t;
    using edge_type = typename GRAPH::edge_type;
    using adj_vertex_iter = KtAdjIter<graph_type>;
    using const_edge_ref = decltype(std::declval<adj_vertex_iter>().edge());
    constexpr static vertex_index_t null_vertex = -1;

    constexpr static bool trace_multi_edges = !GRAPH::isDigraph() && GRAPH::isMultiEdges(); // 对于无向平行图，dfs须追踪平行边
    using tracing_map = std::map<vertex_index_t, std::multimap<vertex_index_t, edge_type>>;
    using tracing_type = std::conditional_t<trace_multi_edges, tracing_map, void*>;
    using const_from_iter = typename tracing_map::const_iterator;
    using const_to_iter = typename std::multimap<vertex_index_t, edge_type>::const_iterator;


    // graph -- 待遍历的图
    // startVertex -- 遍历的起始顶点，-1表示只构建迭代器，需要另外调用start方法开始遍历
    KtDfsIter(GRAPH& graph, vertex_index_t startVertex)
        : graph_(graph)
        , v_(null_vertex)
        , pushOrd_(graph.order(), null_vertex)
        , popOrd_(graph.order(), null_vertex)
        , pushIdx_(0)
        , popIdx_(0) {
        if (startVertex != null_vertex) 
            start(startVertex);

        if constexpr (trace_multi_edges)
            collectMultiEdges_();
    }

    void operator++() {
        assert(!isEnd());

        if (isPopping()) { // 处理出栈顶点
            assert(popOrd_[v_] == null_vertex);
            popOrd_[v_] = popIdx_++;
            todo_.pop_back();
        }
        else {
            if (todo_.size() > 1) {
                if constexpr (trace_multi_edges) 
                    markMultiEdge_();

                ++todo_.back();
            }
            if (isPushing()) {
                pushOrd_[v_] = pushIdx_++;
                todo_.push_back(adj_vertex_iter(graph_, v_));
            }
        }
        
        fixStack_();
    }

    // 返回当前正在游历的顶点
    vertex_index_t operator*() const { return v_; }


    // 与当前顶点（to顶点）构成边的from顶点
    vertex_index_t from() const {
        assert(!isEnd());
        return isPopping() ? grandpa_() :
                            todo_.size() > 1 ? todo_.back().from() : null_vertex;
    }


    // 返回边(from, to)的值
    const_edge_ref edge() const {
        assert(!isEnd() && from() != null_vertex);
        return todo_.back().edge();
    }


    bool isEnd() const { return v_ == null_vertex; }


    // 从顶点v开始接续进行广度优先遍历
    void start(vertex_index_t v) {
        assert(isEnd() && pushOrd_[v] == null_vertex);
        todo_.clear();
        todo_.push_back(adj_vertex_iter(graph_));
        v_ = v;
        if (modeEdge && !stopAtPopping) 
            ++(*this); // skip v0
    }


    void erase() {
        assert(!isEnd() && from() != null_vertex);

        if (isPopping()) { // 处理出栈顶点
            assert(popOrd_[v_] == null_vertex);
            popOrd_[v_] = popIdx_++;
            todo_.pop_back();
        }

        todo_.back().erase();

        fixStack_();
    }


    /*
    BGL中关于树边、回边、下边、跨边的定义：
    - Tree edges are edges in the search tree(or forest) constructed by running a graph search algorithm over a graph. 
      An edge(u, v) is a tree edge if v was first discovered while exploring edge(u, v).
    - Back edges connect vertices to their ancestors in a search tree. 
      So for edge(u, v) the vertex v must be the ancestor of vertex u.
      Self loops are considered to be back edges.
    - Forward edges are non-tree edges(u, v) that connect a vertex u to a descendant v in a search tree.
    - Cross edges are edges that do not fall into the above three categories.
    */

    // 树边，表示递归调用（即第一次访问该节点）
    bool isTree() const {
        return pushOrd_[v_] == null_vertex;
    }


    // 回边，表示当前节点是前序节点的祖先
    bool isBack() const {
        return !isTree() && !isPopping() && popOrd_[v_] == null_vertex;
    }


    // 下边/前边，表示当前节点是前序节点的子孙
    bool isDown() const {
        //return !isTree() && !isBack() && pushOrd_[**this] > pushOrd_[from()];
        assert(pushOrd_[from()] != null_vertex);
        return static_cast<int>(pushOrd_[v_]) > static_cast<int>(pushOrd_[from()]);
    }


    // 跨边，表示当前节点既不是前序节点的祖先，也不是子孙
    bool isCross() const {
        //return !isTree() && !isBack() && !isDown();
        return GRAPH::isDigraph() && popOrd_[v_] != null_vertex; // 只有有向图才有跨边
    }

    // 当前节点是否正在入栈，对应于递归的入口
    bool isPushing() const { return isTree(); }

    // 当前节点是否正在出栈，对应于递归的出口
    bool isPopping() const { 
        return stopAtPopping && !isPushing() && todo_.back().isEnd(); 
    }

    // 获取顶点v的入栈/出栈次序，用于事后检测
    unsigned pushIndex(vertex_index_t v) const { return pushOrd_[v]; }
    unsigned popIndex(vertex_index_t v) const { return popOrd_[v]; }

    // 获取当前的入栈/出栈序号
    unsigned pushingIndex() const { return pushIdx_; }
    unsigned poppingIndex() const { return popIdx_; }


    vertex_index_t firstUnvisited() const {
        auto pos = std::find(pushOrd_.begin(), pushOrd_.end(), null_vertex);
        return pos == pushOrd_.end() ? null_vertex
            : static_cast<vertex_index_t>(std::distance(pushOrd_.begin(), pos));
    }


private:

    // 返回当前顶点的祖父顶点，即from之from
    vertex_index_t grandpa_() const {
        return todo_.size() > 2 ? todo_[todo_.size() - 2].from() : null_vertex;
    }


    // 步进或删除的后处理
    void fixStack_();

    // 测试是否需要跳过当前顶点/边
    bool testSkip_() const;

    // 搜集graph_中的所有平行边
    template<bool dummy = trace_multi_edges, typename = std::enable_if_t<dummy>>
    void collectMultiEdges_();
      
    // 标记平行边<from, to, wt>已遍历
    template<bool dummy = trace_multi_edges, typename = std::enable_if_t<dummy>>
    void markMultiEdge_();

    // 测试平行边<from, to, wt>遍历状态，返回true表示已遍历
    template<bool dummy = trace_multi_edges, typename = std::enable_if_t<dummy>>
    bool testMultiEdge_() const;

    // 辅助函数：定位当前边在graph_中的位置
    template<bool dummy = trace_multi_edges, typename = std::enable_if_t<dummy>>
    std::pair<const_from_iter, const_to_iter>  findMultiEdges_() const {
        auto adj = todo_.back();
        auto v = adj.from(), w = *adj;
        if (v > w) std::swap(v, w);

        auto iter = medges_.find(v);
        if (iter == medges_.end())
            return { iter, {} };

        auto r = iter->second.equal_range(w);
        auto pos = iter->second.cend();

        if (r.first != r.second && r.first->first == w)
            for (pos = r.first; pos != r.second; ++pos)
                if (pos->second == adj.edge())
                   break;

        return { iter, pos };
    }

private:
    graph_type& graph_;

    // 待处理的邻接顶点迭代器
    std::vector<adj_vertex_iter> todo_;

    vertex_index_t v_; // 正在遍历的顶点

    std::vector<unsigned> pushOrd_, popOrd_; // 用于记录各顶点的压栈/出栈顺序
    unsigned pushIdx_, popIdx_; // 当前压栈/出栈序号

    tracing_type medges_; // 存储graph_所有未遍历的平行边
};


template<typename GRAPH, bool fullGraph, bool modeEdge, bool stopAtPopping>
void KtDfsIter<GRAPH, fullGraph, modeEdge, stopAtPopping>::fixStack_()
{
    // 检测当前顶点是否需要跳过
    while (todo_.size() > 1) {
        auto& iter = todo_.back();

        // 移除已结束的迭代器
        if (iter.isEnd()) {
            if constexpr (stopAtPopping) 
                break; // stop at popping
 
            popOrd_[iter.from()] = popIdx_++;
            todo_.pop_back();
            continue;
        }

        assert(!isPopping() && !iter.isEnd());

        if (testSkip_()) {
            ++iter;
            continue;
        }

        break;
    }

    // 更新v_和接续遍历
    if (todo_.size() <= 1) {
        v_ = null_vertex; // 设置终止标记

        if constexpr (fullGraph) {
            unsigned unvisted = firstUnvisited();
            if (unvisted != null_vertex)
                start(unvisted); // 接续遍历
        }
    }
    else {
        auto& iter = todo_.back();
        v_ = iter.isEnd() ? iter.from() : *iter;
    }
}


template<typename GRAPH, bool fullGraph, bool modeEdge, bool stopAtPopping>
bool KtDfsIter<GRAPH, fullGraph, modeEdge, stopAtPopping>::testSkip_() const
{
    auto& iter = todo_.back();

    // 防止无向图的顶点回溯，即防止已遍历的无向边(v, w)再次通过(w, v)遍历
    if (!GRAPH::isDigraph() && *iter == grandpa_())
        if constexpr (trace_multi_edges)
            return testMultiEdge_();
        else
            return true;

    if (modeEdge) {
        if (!GRAPH::isDigraph() && popOrd_[*iter] != null_vertex)
            return true; // 对于无向图，若某顶点已出栈，则与之邻接的边必然已遍历
    }
    else if (pushOrd_[*iter] != null_vertex)
        return true; // 跳过已遍历的顶点，确保每个顶点只遍历一次

    return false;
}


template<typename GRAPH, bool fullGraph, bool modeEdge, bool stopAtPopping>
template<bool dummy, typename>
void KtDfsIter<GRAPH, fullGraph, modeEdge, stopAtPopping>::collectMultiEdges_()
{
    assert(medges_.empty());
    for (vertex_index_t v = 0; v < graph_.order(); v++) {

        auto& edgeMap = medges_[v];
        for (adj_vertex_iter iter(graph_, v); !iter.isEnd(); ++iter)
            if(*iter > v) // 只保存to顶点大于from顶点的边，忽略自环
                edgeMap.insert({ *iter, iter.edge() });
       
        for (auto iter = edgeMap.cbegin(); iter != edgeMap.cend();) {
            if (edgeMap.count(iter->first) == 1)
                iter = edgeMap.erase(iter); // 删除非平行边
            else
                ++iter;
        }

        if (edgeMap.empty())
            medges_.erase(v);
    }
}


template<typename GRAPH, bool fullGraph, bool modeEdge, bool stopAtPopping>
template<bool dummy, typename>
void KtDfsIter<GRAPH, fullGraph, modeEdge, stopAtPopping>::markMultiEdge_()
{
    auto pos = findMultiEdges_();
    if (pos.first != medges_.cend()
        && pos.second != pos.first->second.cend()) {
        unsigned v = pos.first->first;
        auto iter = medges_[v].erase(pos.second);
        if (iter == medges_[v].end())
            medges_.erase(v);
    }
}


template<typename GRAPH, bool fullGraph, bool modeEdge, bool stopAtPopping>
template<bool dummy, typename>
bool KtDfsIter<GRAPH, fullGraph, modeEdge, stopAtPopping>::testMultiEdge_() const
{
    auto pos = findMultiEdges_();
    return pos.first == medges_.cend() || pos.second == pos.first->second.cend();
}
