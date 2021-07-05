#pragma once
#include <vector>
#include <assert.h>
#include "KtGraphBase.h"


// ͳһ����ͼ������ͼ��������ȱ���ģ���ܣ�ͨ��ģ�������֧�ֶ��ֱ�����ʽ��
// ģ�������
//    -- stopAtPopping����Ϊtrue����ÿ�������ջʱ��������ͣ���û��л��ᴦ���ջ����.
//    -- fullGraph���μ�KtBfsIter.
//    -- modeEdge���μ�KtBfsIter.
// ��modeEdgeΪtrue�� stopAtPoppingΪfalseʱ����ʵ��Ϊ�ߵ���������ʼ���㱻�������ѱ�֤��ʼ״̬��Ϊ��һ����
// ��modeEdgeΪtrue�� stopAtPoppingΪtrueʱ����ʼ״̬�Է�����ʼ���㣬��ʱfrom()����-1.
template<typename GRAPH, bool fullGraph = false, bool modeEdge = false, bool stopAtPopping = false>
class KtDfsIter
{
    using adj_vertex_iter = typename GRAPH::adj_vertex_iter;

public:

    // graph -- ��������ͼ
    // startVertex -- ��������ʼ���㣬-1��ʾֻ��������������Ҫ�������begin������ʼ����
    KtDfsIter(const GRAPH& graph, unsigned startVertex)
        : graph_(graph),
        v_(-1),
        pushOrd_(graph.order(), -1),
        popOrd_(graph.order(), -1),
        pushIdx_(0), popIdx_(0) {
        if (startVertex != -1) begin(startVertex);
    }

    void operator++() {
        assert(!isEnd());

        if (isPopping()) { // �����ջ����
            assert(popOrd_[v_] == -1);
            popOrd_[v_] = popIdx_++;
            todo_.pop_back();
        }
        else {
            if (todo_.size() > 1)
                ++todo_.back().first;
            if (isPushing()) {
                pushOrd_[v_] = pushIdx_++;
                todo_.emplace_back(adj_vertex_iter{ graph_, v_ }, v_);
            }
        }


        // ��⵱ǰ�����Ƿ���Ҫ����
        while (todo_.size() > 1) {
            auto& iter = todo_.back().first;

            // �Ƴ��ѽ����ĵ�����
            if (iter.isEnd()) {
                if (!stopAtPopping) {
                    popOrd_[todo_.back().second] = popIdx_++;
                    todo_.pop_back();
                    continue;
                }

                break; // stop at popping
            }


            assert(!isPopping() && !iter.isEnd());

            // ��ֹ����ͼ�Ķ������
            if (!GRAPH::isDigraph() && *iter == grandpa()) {
                ++iter;
                continue;
            }

            // �����ѱ����Ķ�����
            bool needSkip = false;
                
            if (modeEdge) {
                if (!GRAPH::isDigraph() && popOrd_[*iter] != -1) // ��������ͼ����ĳ�����ѳ�ջ������֮�ڽӵı߱�Ȼ�ѱ���
                    needSkip = true;
            }
            else if (pushOrd_[*iter] != -1)  // �����ѱ����Ķ��㣬ȷ��ÿ������ֻ����һ��
                    needSkip = true;

            if (needSkip) {
                ++iter;
                continue;
            }
        
            break;
        }

        // ����v_�ͽ�������
        if (todo_.size() <= 1) {
            v_ = -1; // ������ֹ���

            if (fullGraph) {
                unsigned unvisted = firstUnvisited();
                if(unvisted != -1) begin(unvisted); // ��������
            }
        }
        else {
            v_ = todo_.back().first.isEnd() ? todo_.back().second : *todo_.back().first;
        }
    }

    // ���ص�ǰ���������Ķ���
    unsigned operator*() const { return v_; }


    // �뵱ǰ���㣨to���㣩���ɱߵ�from����
    unsigned from() const {
        assert(!isEnd());
        return isPopping() ? grandpa() :
                            todo_.size() > 1 ? todo_.back().second : -1;
    }


    // ���ر�(from, to)��ֵ
    auto value() const {
        assert(!isEnd() && from() != -1);
        return todo_.back().first.value();
    }


    bool isEnd() const { return v_ == -1; }


    // �Ӷ���v��ʼ�������й�����ȱ���
    void begin(unsigned v) {
        assert(isEnd() && pushOrd_[v] == -1);
        todo_.clear();
        todo_.emplace_back(adj_vertex_iter(graph_), v_ = v);
        if (modeEdge && !stopAtPopping) 
            ++(*this); // skip v0
    }


    // ���ߣ���ʾ�ݹ���ã�����һ�η��ʸýڵ㣩
    bool isTree() const {
        return pushOrd_[v_] == -1;
    }


    // �رߣ���ʾ��ǰ�ڵ���ǰ��ڵ������
    bool isBack() const {
        return !isTree() && !isPopping() && popOrd_[v_] == -1;
    }


    // �±ߣ���ʾ��ǰ�ڵ���ǰ��ڵ������
    bool isDown() const {
        //return !isTree() && !isBack() && pushOrd_[**this] > pushOrd_[from()];
        assert(pushOrd_[from()] != -1);
        return static_cast<int>(pushOrd_[v_]) > static_cast<int>(pushOrd_[from()]);
    }


    // ��ߣ���ʾ��ǰ�ڵ�Ȳ���ǰ��ڵ�����ȣ�Ҳ��������
    bool isCross() const {
        //return !isTree() && !isBack() && !isDown();
        return GRAPH::isDigraph() && popOrd_[v_] != -1; // ֻ������ͼ���п��
    }

    // ��ǰ�ڵ��Ƿ�������ջ����Ӧ�ڵݹ�����
    bool isPushing() const { return isTree(); }

    // ��ǰ�ڵ��Ƿ����ڳ�ջ����Ӧ�ڵݹ�ĳ���
    bool isPopping() const { return stopAtPopping && !isPushing() && todo_.back().first.isEnd(); }

    unsigned pushIndex(unsigned v) const { return pushOrd_[v]; }
    unsigned popIndex(unsigned v) const { return popOrd_[v]; }

    unsigned pushingIndex() const { return pushIdx_; }
    unsigned poppingIndex() const { return popIdx_; }


    unsigned firstUnvisited() const {
        auto pos = std::find(pushOrd_.begin(), pushOrd_.end(), -1);
        return pos == pushOrd_.end() ? -1 : std::distance(pushOrd_.begin(), pos);
    }


    unsigned graphOrder() const {
        return graph_.order();
    }

private:

    // ���ص�ǰ������游���㣬��from֮from
    unsigned grandpa() const {
        return todo_.size() > 2 ? todo_[todo_.size() - 2].second : -1;
    }

private:
    const GRAPH& graph_;

    // ��������ڽӶ����������ʹ��pair�ṹ����Ҫ��Ϊ�˷����Чʵ��from����
    std::vector<std::pair<adj_vertex_iter, unsigned>> todo_;

    unsigned v_; // ���ڱ����Ķ���

    std::vector<unsigned> pushOrd_, popOrd_; // ���ڼ�¼�������ѹջ/��ջ˳��
    unsigned pushIdx_, popIdx_; // ��ǰѹջ/��ջ���
};

