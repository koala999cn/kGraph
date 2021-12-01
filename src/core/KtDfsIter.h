#pragma once
#include <vector>
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
    using adj_vertex_iter = decltype(std::declval<graph_type>().adjIter(0));
    using const_edge_ref = decltype(std::declval<adj_vertex_iter>().edge());
    enum { null_vertex = GRAPH::null_vertex };


    // graph -- 待遍历的图
    // startVertex -- 遍历的起始顶点，-1表示只构建迭代器，需要另外调用begin方法开始遍历
    KtDfsIter(GRAPH& graph, vertex_index_t startVertex)
        : graph_(graph),
        v_(null_vertex),
        pushOrd_(graph.order(), null_vertex),
        popOrd_(graph.order(), null_vertex),
        pushIdx_(0), popIdx_(0) {
        if (startVertex != null_vertex) begin(startVertex);
    }

    void operator++() {
        assert(!isEnd());

        if (isPopping()) { // 处理出栈顶点
            assert(popOrd_[v_] == null_vertex);
            popOrd_[v_] = popIdx_++;
            todo_.pop_back();
        }
        else {
            if (todo_.size() > 1)
                ++todo_.back();
            if (isPushing()) {
                pushOrd_[v_] = pushIdx_++;
                todo_.push_back(graph_.adjIter(v_));
            }
        }
        
        advancePost_();
    }

    // 返回当前正在游历的顶点
    vertex_index_t operator*() const { return v_; }


    // 与当前顶点（to顶点）构成边的from顶点
    vertex_index_t from() const {
        assert(!isEnd());
        return isPopping() ? grandpa() :
                            todo_.size() > 1 ? todo_.back().other() : null_vertex;
    }


    // 返回边(from, to)的值
    const_edge_ref edge() const {
        assert(!isEnd() && from() != null_vertex);
        return todo_.back().edge();
    }


    bool isEnd() const { return v_ == null_vertex; }


    // 从顶点v开始接续进行广度优先遍历
    void begin(vertex_index_t v) {
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

        advancePost_();
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
        return pushOrd_[v_] == -1;
    }


    // 回边，表示当前节点是前序节点的祖先
    bool isBack() const {
        return !isTree() && !isPopping() && popOrd_[v_] == -1;
    }


    // 下边/前边，表示当前节点是前序节点的子孙
    bool isDown() const {
        //return !isTree() && !isBack() && pushOrd_[**this] > pushOrd_[from()];
        assert(pushOrd_[from()] != -1);
        return static_cast<int>(pushOrd_[v_]) > static_cast<int>(pushOrd_[from()]);
    }


    // 跨边，表示当前节点既不是前序节点的祖先，也不是子孙
    bool isCross() const {
        //return !isTree() && !isBack() && !isDown();
        return GRAPH::isDigraph() && popOrd_[v_] != -1; // 只有有向图才有跨边
    }

    // 当前节点是否正在入栈，对应于递归的入口
    bool isPushing() const { return isTree(); }

    // 当前节点是否正在出栈，对应于递归的出口
    bool isPopping() const { return stopAtPopping && !isPushing() && todo_.back().isEnd(); }

    unsigned pushIndex(unsigned v) const { return pushOrd_[v]; }
    unsigned popIndex(unsigned v) const { return popOrd_[v]; }

    unsigned pushingIndex() const { return pushIdx_; }
    unsigned poppingIndex() const { return popIdx_; }


    unsigned firstUnvisited() const {
        auto pos = std::find(pushOrd_.begin(), pushOrd_.end(), -1);
        return pos == pushOrd_.end() ? -1 : std::distance(pushOrd_.begin(), pos);
    }


    graph_type& graph() { return graph_; }


private:

    // 返回当前顶点的祖父顶点，即from之from
    vertex_index_t grandpa() const {
        return todo_.size() > 2 ? todo_[todo_.size() - 2].other() : null_vertex;
    }


    // 步进或删除的后处理
    void advancePost_() {

        // 检测当前顶点是否需要跳过
        while (todo_.size() > 1) {
            auto& iter = todo_.back();

            // 移除已结束的迭代器
            if (iter.isEnd()) {
                if (!stopAtPopping) {
                    popOrd_[todo_.back().other()] = popIdx_++;
                    todo_.pop_back();
                    continue;
                }

                break; // stop at popping
            }


            assert(!isPopping() && !iter.isEnd());

            // 防止无向图的顶点回溯
            if (!GRAPH::isDigraph() && *iter == grandpa()) {
                ++iter;
                continue;
            }

            // 跳过已遍历的顶点或边
            bool needSkip = false;

            if (modeEdge) {
                if (!GRAPH::isDigraph() && popOrd_[*iter] != -1) // 对于无向图，若某顶点已出栈，则与之邻接的边必然已遍历
                    needSkip = true;
            }
            else if (pushOrd_[*iter] != -1)  // 跳过已遍历的顶点，确保每个顶点只遍历一次
                needSkip = true;

            if (needSkip) {
                ++iter;
                continue;
            }

            break;
        }

        // 更新v_和接续遍历
        if (todo_.size() <= 1) {
            v_ = null_vertex; // 设置终止标记

            if (fullGraph) {
                unsigned unvisted = firstUnvisited();
                if (unvisted != null_vertex) begin(unvisted); // 接续遍历
            }
        }
        else {
            v_ = todo_.back().isEnd() ? todo_.back().other() : *todo_.back();
        }
    }

private:
    graph_type& graph_;

    // 待处理的邻接顶点迭代器
    std::vector<adj_vertex_iter> todo_;

    vertex_index_t v_; // 正在遍历的顶点

    std::vector<unsigned> pushOrd_, popOrd_; // 用于记录各顶点的压栈/出栈顺序
    unsigned pushIdx_, popIdx_; // 当前压栈/出栈序号
};

