#pragma once
#include <vector>
#include <algorithm>


// ϡ������ʵ�֣�����CSR��Compressed Sparse Row���㷨
template<typename T>
class KtSparseMatrix
{
public:
    typedef T value_type;
    typedef std::pair<unsigned, value_type> element_type;
    typedef typename std::vector<element_type>::iterator element_iterator;
    typedef typename std::vector<element_type>::const_iterator const_element_iterator;

    KtSparseMatrix() : cols_(0), default_(0) {}
    KtSparseMatrix(const KtSparseMatrix&) = default;
    KtSparseMatrix(KtSparseMatrix&&) = default;
    KtSparseMatrix& operator=(const KtSparseMatrix&) = default;
    KtSparseMatrix& operator=(KtSparseMatrix&&) = default;

    KtSparseMatrix(unsigned rows, unsigned cols, const T& defaultVal) : 
        cols_(cols), default_(defaultVal), row_offsets_(rows + 1, -1) {   
            row_offsets_.back() = 0; // ���һ��Ԫ�ر�ʾ��Ч������������ʼΪ0
    }

    void reset(unsigned rows, unsigned cols, const value_type& val) {
        cols_ = cols, default_ = val;
        row_offsets_.assign(rows + 1, -1);
        row_offsets_.back() = 0;
        elements_.clear();
    }
    
    const value_type& operator()(unsigned row, unsigned col) const {
        const_element_iterator x = const_cast<KtSparseMatrix<value_type>*>(this)->getAt(row, col, false);
        return x == elements_.end() ? default_ : x->second;
    }

    value_type& operator()(unsigned row, unsigned col) {
        auto x = getAt(row, col, true);
        return x->second;
    }

	unsigned rows() const { return row_offsets_.size() - 1; }
	unsigned cols() const { return cols_; }


    // ��ȡ��row����Ч���ݵ�����
    unsigned getSizeOfRow(unsigned row) const {
        unsigned begin, end;
        rowRange(row, &begin, &end, false);

        return begin == -1 ? 0 : end - begin;
    }

    // ����row�С���col��������Ϊdefault��ʵ�ʵ�Ч��ɾ������
    void setDefault(unsigned row, unsigned col) {
        unsigned begin_idx, end_idx;
        rowRange(row, &begin_idx, &end_idx, false);
        if(begin_idx != -1) {
            auto begin_iter = std::next(elements_.begin(), begin_idx);
            auto end_iter = std::next(elements_.begin(), end_idx);
            auto erase_iter = std::lower_bound(begin_iter, end_iter, element_type{col, value_type(0)}, 
                        [](const element_type& a, const element_type& b){ return a.first < b.first; });
            if(erase_iter != end_iter && erase_iter->first == col) { // bingo
                // ɾ������
               elements_.erase(erase_iter);

                // ������ƫ��
                if(end_idx - begin_idx == 1) row_offsets_[row] = -1; 
                while(++row < row_offsets_.size())
                    if(row_offsets_[row] != -1) row_offsets_[row]--; 
            }
        }
    }


    auto rowBegin(unsigned row) const {
        auto begin_idx = row_offsets_[row];
        return begin_idx == -1 ? elements_.end() : std::next(elements_.begin(), begin_idx);
    }

    auto rowEnd(unsigned row) const {
        unsigned begin_idx, end_idx;
        rowRange(row, &begin_idx, &end_idx, false);
        return end_idx == -1 ? elements_.end() : std::next(elements_.begin(), end_idx);
    }


	void eraseRow(unsigned row) { 
		unsigned begin_idx, end_idx;
		rowRange(row, &begin_idx, &end_idx, false);
		row_offsets_.erase(std::next(row_offsets_.begin(), row));
		if (begin_idx != -1) {
			elements_.erase(std::next(elements_.begin(), begin_idx), std::next(elements_.begin(), end_idx));

			// ����row_offsets_
			auto iter = std::next(row_offsets_.begin(), row);
			for (; iter != row_offsets_.end(); ++iter) {
				unsigned i = *iter;
				if(i != -1) *iter = i + offset;
			}			
		}
	}

	void eraseCol(unsigned col) {
		for (unsigned row = 0; row < rows(); row++)
			setDefault(row, col);

		// ����elements_�е�������
		auto iter = elements_.begin();
		for (; iter != elements_.end(); ++iter) {
			assert(iter->first != col);
			if (iter->first > col)
				iter->first--;
		}

		--cols_;
	}


private:

    // ���ص�row�С���col�����ݶ�д��ַ������������insert���棬�򷵻�null
    element_iterator getAt(unsigned row, unsigned col, bool insert) {
        unsigned begin_idx, end_idx;
        rowRange(row, &begin_idx, &end_idx, insert);

        element_iterator insert_iter;

        // ���������Ƿ����
        if(begin_idx != -1) {
            auto begin_iter = elements_.begin() + begin_idx;
            auto end_iter = elements_.begin() + end_idx;
            insert_iter = std::lower_bound(begin_iter, end_iter, element_type{col, value_type(0)}, 
                        [](const element_type& a, const element_type& b){ return a.first < b.first; });
            if(insert_iter != end_iter && insert_iter->first == col) // bingo
                return insert_iter;
        }
        
        if(!insert) return elements_.end();

        if(begin_idx == -1) {
            assert(end_idx != -1);
            insert_iter = elements_.begin() + end_idx;
            row_offsets_[row] = end_idx; 
        }

        auto iter = elements_.insert(insert_iter, { col, default_ });

        // ������ƫ��
        while(++row < row_offsets_.size())
            if(row_offsets_[row] != -1) row_offsets_[row]++; 

        return iter;
    }


    // �����row��λ��elements_�����ݷ�Χ��������ص�begin��end.
    // ���forceEndΪ�棬��*begin == -1ʱ����Ȼ����*endֵ
    void rowRange(unsigned row, unsigned* begin, unsigned* end, bool forceEnd) const {
        *begin = row_offsets_[row];
        if(*begin != -1 || forceEnd) {
            *end = row_offsets_[++row];
            while(*end == -1) *end = row_offsets_[++row];
        }
        else {
            *end = -1;
        }
    }


private:
    value_type default_;
    unsigned cols_;
    std::vector<element_type> elements_; // ��Ĭ�����ݵ�{������������ֵ}����
    std::vector<unsigned> row_offsets_; // ��С����rows+1�����һ��Ԫ�ش洢��Ч��������
};

