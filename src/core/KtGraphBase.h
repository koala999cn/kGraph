#pragma once
#include <vector>
#include <cassert>


// �����ڽӾ����ͼ����
// MAT_TYPEΪ�ڽӾ������ͣ�����Ҫ������3���Ա��
//     1.����������value_type
//     2.���캯����MAT_TYPE(unsigned numRow, unsigned numCol, value_type initValue)
//     3.�������أ�value_type& operaotr(unsigned idxRow, unsigned idxCol)
// directionΪ���������ͼ�������������ͼ��
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

    // ����ͼΪnumVertex���㣬��������б�
    void reset(unsigned numVertex, value_type nullValue = value_type(0)) {
        V_ = numVertex, E_ = 0;
        null_ = nullValue;
        adjMat_.reset(numVertex, numVertex, null_);
    }

    // ����ͼ�Ľף���������Ŀ
    unsigned order() const { return V_; }

    // ����ͼ�ı���
    unsigned size() const { return E_; }

    // ��ͼ
    bool isEmpty() const  { return order() == 0; }

    // ƽ��ͼ
    bool isTrivial() const { return order() == 1; }

    // ��ͼ
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


	// ����
	virtual unsigned outdegree(unsigned v) const {
		unsigned d(0);
		for (unsigned i = 0; i < order(); i++)
			if (hasEdge(v, i)) ++d;
		return d;
	}


	// ���
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

	// ��
	unsigned degree(unsigned v) const {
		auto d = outdegree(v);
		if (direction) d += indegree(v);
		return d;
	}


	// ɾ������v�����ڱ�
	void eraseVertex(unsigned v) {
		auto d = degree(v);
		if (direction && hasEdge(v, v)) --d;
		adjMat_.eraseRow(v);
		adjMat_.eraseCol(v);
		--V_;
		E_ -= d;
	}


protected:
    MAT_TYPE adjMat_; // �ڽӾ���
    value_type null_; // ����Ԫ�ص���null_ʱ������������
    unsigned V_; // ��������
    unsigned E_; // ������
};
