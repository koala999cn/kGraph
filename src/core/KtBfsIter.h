#pragma once
#include <queue>
#include <assert.h>


// 广度优先遍历
// 模板参数：
//    -- fullGraph，若为true，则进行全图迭代，否则仅迭代与v相联通的节点或边。
//    -- modeEdge，若为true，则对边进行广度优先遍历，这时顶点可能会多次重复，但每条边只会遍历一次。
//                 若为false，则对顶点进行广度优先遍历，这时不能保证每条边都会遍历到或只遍历一次。
// 非modeEdge模式下的初始状态，operator*()返回startVertex，from()方法返回-1
// modeEdge模式下的初始状态，operator*()返回startVertex的第一个邻接顶点，from()方法返回startVertex
template <typename GRAPH, bool fullGraph = false, bool modeEdge = false>
class KtBfsIter 
{
    using adj_vertex_iter = typename GRAPH::adj_vertex_iter;

public:

	// graph -- 待遍历的图
	// startVertex -- 遍历的起始顶点，-1表示只构建迭代器，需要另外调用begin方法开始遍历
    KtBfsIter(const GRAPH& graph, unsigned startVertex)
        : graph_(graph),
          v0_(-1),
          isPushed_(graph.order(), false),
          isPopped_(graph.order(), false) {
        if (startVertex != -1) begin(startVertex);
    }

    void operator++() {
        assert(!isEnd());

        if (!isPushed_[v0_]) {
            assert(todo_.empty());
            isPushed_[v0_] = true;
            todo_.emplace(adj_vertex_iter{graph_, v0_}, v0_);
        } else {
            assert(!todo_.empty());
            auto& iter = todo_.front().first;
            unsigned v = *iter;
            if (!isPushed_[v]) {
                isPushed_[v] = true;
                todo_.emplace(adj_vertex_iter{graph_, v}, v);
            }
            ++iter;
        }

        while (!todo_.empty()) {
            auto& iter = todo_.front().first;

            // 移出已到末尾的迭代器
            if (iter.isEnd()) {
                isPopped_[todo_.front().second] = true;
                todo_.pop();
                continue;
            }

            // 跳过已遍历的顶点或边
            if (modeEdge) {
                if (!GRAPH::isDigraph() && isPopped_[*iter]) { // 对于无向图，若某顶点已出栈，则与之邻接的边必然已遍历
                    ++iter;
                    continue;
                }
            } 
            else {
                if (isPushed_[*iter]) { // 跳过已遍历的顶点，确保每个顶点只遍历一次
                    ++iter;
                    continue;
                }
            }

            break;
        }

        if (todo_.empty()) {
            isPopped_[v0_] = true;
            v0_ = -1;  // 设置终止标记
        }

        if (fullGraph && isEnd()) {
            auto pos = std::find(isPushed_.begin(), isPushed_.end(), false);
            if (pos != isPushed_.end()) begin(std::distance(isPushed_.begin(), pos));
        }
    }

    // 返回当前正在游历的顶点
    unsigned operator*() const {
        return todo_.empty() ? v0_ : *(todo_.front().first);
    }


	// 与当前顶点（to顶点）构成边的from顶点
    unsigned from() const {
        assert(!isEnd());
        return todo_.empty() ? -1 : todo_.front().second;
    }


	auto value() const {
		return todo_.front().first.value();
	}


    bool isEnd() const { return v0_ == -1; }


    // 从顶点v开始接续进行广度优先遍历
    void begin(unsigned v) {
        assert(isEnd() && !isPushed_[v]);
        v0_ = v;

		if (modeEdge) ++(*this); // skip v0
    }


	bool isPushed(unsigned v) const { return isPushed_[v]; }
	bool isPopped(unsigned v) const { return isPopped_[v]; }

private:
    const GRAPH& graph_;

	// 待处理的邻接顶点迭代器。使用pair结构，主要是为了方便高效实现from方法
    std::queue<std::pair<adj_vertex_iter, unsigned>> todo_;

    unsigned v0_; // 起始顶点
    std::vector<bool> isPushed_, isPopped_;  // 用于标记顶点是否已压栈/出栈
};
