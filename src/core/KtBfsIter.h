#pragma once
#include <queue>
#include <assert.h>


// ������ȱ���
// ģ�������
//    -- fullGraph����Ϊtrue�������ȫͼ�����������������v����ͨ�Ľڵ��ߡ�
//    -- modeEdge����Ϊtrue����Ա߽��й�����ȱ�������ʱ������ܻ����ظ�����ÿ����ֻ�����һ�Ρ�
//                 ��Ϊfalse����Զ�����й�����ȱ�������ʱ���ܱ�֤ÿ���߶����������ֻ����һ�Ρ�
// ��modeEdgeģʽ�µĳ�ʼ״̬��operator*()����startVertex��from()��������-1
// modeEdgeģʽ�µĳ�ʼ״̬��operator*()����startVertex�ĵ�һ���ڽӶ��㣬from()��������startVertex
template <typename GRAPH, bool fullGraph = false, bool modeEdge = false>
class KtBfsIter 
{
    using adj_vertex_iter = typename GRAPH::adj_vertex_iter;

public:

	// graph -- ��������ͼ
	// startVertex -- ��������ʼ���㣬-1��ʾֻ��������������Ҫ�������begin������ʼ����
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

            // �Ƴ��ѵ�ĩβ�ĵ�����
            if (iter.isEnd()) {
                isPopped_[todo_.front().second] = true;
                todo_.pop();
                continue;
            }

            // �����ѱ����Ķ�����
            if (modeEdge) {
                if (!GRAPH::isDigraph() && isPopped_[*iter]) { // ��������ͼ����ĳ�����ѳ�ջ������֮�ڽӵı߱�Ȼ�ѱ���
                    ++iter;
                    continue;
                }
            } 
            else {
                if (isPushed_[*iter]) { // �����ѱ����Ķ��㣬ȷ��ÿ������ֻ����һ��
                    ++iter;
                    continue;
                }
            }

            break;
        }

        if (todo_.empty()) {
            isPopped_[v0_] = true;
            v0_ = -1;  // ������ֹ���
        }

        if (fullGraph && isEnd()) {
            auto pos = std::find(isPushed_.begin(), isPushed_.end(), false);
            if (pos != isPushed_.end()) begin(std::distance(isPushed_.begin(), pos));
        }
    }

    // ���ص�ǰ���������Ķ���
    unsigned operator*() const {
        return todo_.empty() ? v0_ : *(todo_.front().first);
    }


	// �뵱ǰ���㣨to���㣩���ɱߵ�from����
    unsigned from() const {
        assert(!isEnd());
        return todo_.empty() ? -1 : todo_.front().second;
    }


	auto value() const {
		return todo_.front().first.value();
	}


    bool isEnd() const { return v0_ == -1; }


    // �Ӷ���v��ʼ�������й�����ȱ���
    void begin(unsigned v) {
        assert(isEnd() && !isPushed_[v]);
        v0_ = v;

		if (modeEdge) ++(*this); // skip v0
    }


	bool isPushed(unsigned v) const { return isPushed_[v]; }
	bool isPopped(unsigned v) const { return isPopped_[v]; }

private:
    const GRAPH& graph_;

	// ��������ڽӶ����������ʹ��pair�ṹ����Ҫ��Ϊ�˷����Чʵ��from����
    std::queue<std::pair<adj_vertex_iter, unsigned>> todo_;

    unsigned v0_; // ��ʼ����
    std::vector<bool> isPushed_, isPopped_;  // ���ڱ�Ƕ����Ƿ���ѹջ/��ջ
};
