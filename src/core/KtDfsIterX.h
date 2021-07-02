#pragma once
#include "KtDfsIter.h"


// KtDfsIter�ļ�ǿ�棬����stopAtPopping����Ҫ����2���¹���֧�֣�
//   һ�������������Сǰ����֧�֣�lowֵ������ʾ�ö���Ϊ�����������κλر������õ���Сǰ���ţ�orderֵ��
//   ���������жϵ�ǰ�������丸�����Ƿ�Ϊ�ŵ�֧��
// lowֵ���壺 low(v) = min(pushOrder[v], low[w], pushOrder[k])  
//   ʽ�У�w�Ƕ���v�ĺ��ӽڵ㣬k�Ƕ���v�Ļر����Ƚڵ�
template<typename GRAPH, bool fullGraph = false>
class KtDfsIterX : public KtDfsIter<GRAPH, false, true, true>
{
    using super_ = KtDfsIter<GRAPH, false, true, true>;

public:

    KtDfsIterX(const GRAPH& graph, unsigned v = 0) : super_(graph, v), low_(graph.order(), -1) {
        low_[v] = pushingIndex(); 
    }

	using super_::pushingIndex;
	using super_::pushIndex;
	using super_::from;
	using super_::isEnd;
	using super_::isPushing;
	using super_::isPopping;
	using super_::isBack;
    using super_::firstUnvisited;


    // ��д�����++����������
    void operator++() { 
        super_::operator++();

        if(!isEnd()) {
            unsigned w = **this;
            
            if(isPushing()) {
                assert(pushIndex(w) == -1);
                low_[w] = pushingIndex(); // ��ʼ��low(v) = pushOrder[v]
            }
            else {
				unsigned v = from();
				if (v != -1) {
					if (isBack()) {
						if (low_[v] > pushIndex(w))
							low_[v] = pushIndex(w); // low(v) = min(low[v], pushOrder[k])   
					}
					else if (low_[v] > low_[w])
						low_[v] = low_[w]; // low(v) = min(low[v], low[w]) 
				}
            }  
        }
		else if (fullGraph) {
			unsigned unvisted = firstUnvisited();
			if (unvisted != -1) begin(unvisted);
		}
    }


    // ��д�����begin��������ͬ������lowֵ
    void begin(unsigned v) {
        super_::begin(v);
        low_[v] = pushingIndex();
    }


    unsigned lowIndex(unsigned v) const {
        return low_[v];
    }

    void resetLowIndex(unsigned v) {
        low_[v] = -1;
    }


    bool isBridge() const {
        unsigned v = **this;
        return /*from() != -1 && */isPopping() && lowIndex(v) == pushIndex(v);
    }


private:
    std::vector<unsigned> low_; // �洢���ڵ�չ������ڵ������ӽڵ����С���
};