#pragma once
#include <vector>
#include "KtDfsIterX.h"
#include "KtTopologySort.h"


/* 
  ����һ������ͼ������Ӽ�S�������S����ȡ��������u��v�������ҵ�һ����u��v��·������ôS��ǿ��ͨ�ġ�
  �����ǿ��ͨ�Ķ��㼯��S�м����������ⶥ�㣬����������ǿ��ͨ�ģ���ôS��ԭͼ��һ��ǿ��ͨ������SCC��Strongly Connected Component����
  ��������ͼ�����Էֽ�����ɲ���ɵ�SCC�������ǿ��ͨ�����ֽ⡣
  �ѷֽ���SCC����һ�����㣬�͵õ�һ��DAG��
 */

class KvStronglyConnected
{
public:

    // ����ǿ��ͨ������Ŀ
    unsigned count() const {
        return numScc_;
    }

    // �жϽڵ�v��w�Ƿ�����ͬһǿ��ͨ����
    bool reachable(unsigned v, unsigned w) const {
        return idScc_[v] == idScc_[w];
    }

    // ���ؽڵ�v����ǿ��ͨ������ID
    unsigned operator[](unsigned v) const {
        return idScc_[v];
    }


protected:
    unsigned numScc_; // ǿ��ͨ������Ŀ
    std::vector<unsigned> idScc_; // ǿ��ͨ������id����ͬid�Ķ�������ͬһǿ��ͨ����
};


// ����Kosaraju�㷨��SCC�ֽ�
// �㷨ͨ������򵥵�DFSʵ�֣�
//   ��һ�飺�����ж�����к����š���ɺ�Խ�ӽ�ͼ��β������������Ҷ�ӣ�������ı��ԽС��
//   �ڶ��飺�Ƚ����б߷��򣨻������ͼ��������Ȼ���Ա�����Ķ���Ϊ������DFS���������㼯�ϼ�Ϊһ��SCC��
//          ֮��ֻҪ������δ���ʵĶ��㣬�ʹ���ѡȡ������Ĳ����ظ�DFS��
template<typename GRAPH>
class KtStronglyConnectedKos : public KvStronglyConnected
{
    static_assert(GRAPH::isDigraph(), "KtStronglyConnectedKos must instantiated with DiGraph.");

public:
    KtStronglyConnectedKos(const GRAPH& g) {
        // ��һ��DFS
        KtTopologySortInv<GRAPH> ts(g); // ���������㷨ֻ��DAG��Ч�����Դ˴�ʹ�û���DFS��������������������

        // �ڶ���DFS
        auto gR = g.template reverse<GRAPH>(); // TODO��ʹ����DFS����������ʡȴ������ͼ��ʱ��Ϳռ�
        unsigned V = g.order();
        numScc_ = 0;
        idScc_.assign(V, -1);        
        unsigned i(V-1);
        KtDfsIter<GRAPH> iter(gR, -1);
    
        while(true) {
            iter.begin(ts[i]);

            while(!iter.isEnd()) {
                if(iter.isTree()) {
                    assert(idScc_[*iter] == -1);
                    idScc_[*iter] = numScc_;
                }
                ++iter;      
            }

            ++numScc_;

            // ������δ���ʵ�����Ŷ���
            while(i != 0 && idScc_[ts[--i]] != -1);

            if(idScc_[ts[i]] != -1) // ���ж�����ѷ���
                break;
        }
    }
};


// ����Tarjan�㷨��SCC�ֽ�
// ��findBridge�㷨���ƣ�ͨ��lowֵ�����Ծۺ���ͨ����
template<typename GRAPH>
class KtStronglyConnectedTar : public KvStronglyConnected
{
    static_assert(GRAPH::isDigraph(), "KtStronglyConnectedTar must instantiated with DiGraph.");

public:
    KtStronglyConnectedTar(const GRAPH& g) {
        unsigned V = g.order();
        numScc_ = 0;
        idScc_.resize(V, -1);       
        std::vector<unsigned> S; 
        KtDfsIterX<GRAPH, true> iter(g, 0);
        while(!iter.isEnd()) {
            unsigned v = *iter;
			//printf("v=%d, prev=%d, low=%d, popping=%s\n", v, 
			//	iter.isPushing() ? iter.pushingIndex() : iter.pushIndex(v),
			//	iter.lowIndex(v), iter.isPopping() ? "Y" : "N");
            if(iter.isPushing()) 
                S.push_back(v);

            if(iter.isBridge()) {
                unsigned w;
                do {
                    w = S.back(); S.pop_back();
                    idScc_[w] = numScc_; 
                    iter.resetLowIndex(w); 
                }while(w != v);
                numScc_++;
            } 

            ++iter;
        }
    }
};


// ����Gabow�㷨��SCC�ֽ�
template<typename GRAPH>
class KtStronglyConnectedGab : public KvStronglyConnected
{
    static_assert(GRAPH::isDigraph(), "KtStronglyConnectedGab must instantiated with DiGraph.");

public:
    KtStronglyConnectedGab(const GRAPH& g) {
        unsigned V = g.order();
        numScc_ = 0;
        idScc_.resize(V, -1);       
        std::vector<unsigned> S, path;
        KtDfsIter<GRAPH, true, true, true> iter(g, 0);
        while(!iter.isEnd()) {
            unsigned v = *iter;
            if(iter.isTree()) {
                S.push_back(v);
                path.push_back(v);
            }
            else if(idScc_[v] == -1) {
                assert(iter.pushIndex(v) != -1);
                while(iter.pushIndex(path.back()) > iter.pushIndex(v))
                    path.pop_back();
            }

            if(iter.isPopping() && path.back() == v) {
                path.pop_back();

                unsigned w;
                do {
                    w = S.back(); S.pop_back();
                    idScc_[w] = numScc_;
                }while(w != v);    
                numScc_++;     
            }                

            ++iter;
        }
    }
};