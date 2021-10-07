#pragma once
#include <vector>
#include <algorithm>
#include "KtRange.h"



// 基于vector的稀疏矩阵实现
//  @bMultiVal: 是否允许同一矩阵位置有多个值
template<typename T, bool bMultiVal, typename ROW_CONTAINER = std::vector<std::pair<unsigned, T>>>
class KtSparseMatrix
{
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using element_type = std::pair<unsigned, T>;
    using row_type = ROW_CONTAINER;

	constexpr static bool is_multi_value = bMultiVal;

	using row_element_iter = typename row_type::iterator;
	using const_row_element_iter = typename row_type::const_iterator;
	using row_range = KtRange<row_element_iter>;
	using const_row_range = KtRange<const_row_element_iter>;


private:

	template<bool bConst>
	class col_element_iter_
	{
	public:
		using spare_matrix_type = std::conditional_t<bConst, const KtSparseMatrix, KtSparseMatrix>;
		using row_range_type = std::conditional_t<bConst, const_row_range, row_range>;
		using value_type_ = std::conditional_t<bConst, const value_type, value_type>;
		using deref_type = std::pair<const unsigned, value_type_&>;
		using const_deref_type = const std::pair<const unsigned, const value_type_&>;

		col_element_iter_(spare_matrix_type& mat, row_range_type& range, unsigned rowIdx, unsigned colIdx) :
			mat_(mat), range_(range), curRow_(rowIdx), colIdx_(colIdx) {}

		col_element_iter_& operator++() {
			assert(!range_.empty());
			++range_;
			return next_();
		}


		deref_type operator*() { return{ curRow_, range_->second }; }
		const_deref_type operator*() const { return{ curRow_, range_->second }; }


		bool operator==(const col_element_iter_& rhs) const {
			assert(&mat_ == &rhs.mat_ && colIdx_ == rhs.colIdx_);
			return curRow_ == rhs.curRow_ && range_ == rhs.range_;
		}


		void erase() {
			assert(curRow_ < rows());
			range_.begin() = mat_.elements_[curRow_].erase(range_.begin());
			next_();
		}


	private:
		col_element_iter_& next_() {
			assert(curRow_ < mat_.rows());
			using iter_type = typename row_range_type::iter_type;
			auto colIdx = colIdx_;
			auto pred = [colIdx](const iter_type& iter) { return colIdx == iter->first; };

			if (bMultiVal) { // 多值模式下，顺着当前range继续往下搜索
				range_.skipUntil(pred);
				if (!range_.empty())
					return *this;
			}

			// goto next row
			while (++curRow_ < rows()) {
				range_ = row(curRow_);
				range_.skipUntil(pred);
				if (!range_.empty())
					break; // bingo
			}
			
			return *this;
		}

	private:
		spare_matrix_type& mat_;
		row_range_type range_;
		const unsigned colIdx_; // 当前正在迭代的列号
		unsigned curRow_; // 追踪range_的行号
	};


public:

	using col_element_iter = col_element_iter_<false>;
	using const_col_element_iter = col_element_iter_<true>;
	using col_range = KtRange<col_element_iter>;
	using const_col_range = KtRange<const_col_element_iter>;


    KtSparseMatrix() : cols_{0}, default_{} {}
    KtSparseMatrix(const KtSparseMatrix&) = default;
    KtSparseMatrix(KtSparseMatrix&&) = default;
    KtSparseMatrix& operator=(const KtSparseMatrix&) = default;
    KtSparseMatrix& operator=(KtSparseMatrix&&) = default;

    KtSparseMatrix(unsigned rows, unsigned cols, const_reference defaultVal) :
        cols_(0), default_(defaultVal), elements_(rows) { }

    void resize(unsigned rows, unsigned cols, const_reference val) {
        cols_ = cols;  default_ = val;
        elements_.clear(); elements_.resize(rows);
    }
    
    void reserve(unsigned rows, unsigned cols) { elements_.reserve(rows); }


    const_reference operator()(unsigned row, unsigned col) const {
        auto iter = const_cast<KtSparseMatrix<value_type, bMultiVal>*>(this)->getAt(row, col, false);
        return iter == elements_[row].end() ? default_ : iter->second;
    }


  
    reference operator()(unsigned row, unsigned col) {
        auto iter = getAt(row, col, true);
        assert(iter != elements_[row].end());
        return iter->second;
    }

	reference operator()(unsigned row, unsigned col, const_reference val) {
		if (!bMultiVal) {
			assert((*this)(row, col) == val);
			return (*this)(row, col);
		}

		auto iter = getAt(row, col, false);
		while (iter != elements_[row].end()) {
			if (iter->first == col && iter->second == val) 
				return iter->second;
		}

		assert(false);
		return default_; // make compiler easy.
	}


    unsigned rows() const { return elements_.size(); }
    unsigned cols() const { return cols_; }

    
    // 返回第rowIdx行range对象
    auto row(unsigned rowIdx) { 
		return row_range(elements_[rowIdx].begin(), elements_[rowIdx].end());
	}

    auto row(unsigned rowIdx) const {
		return const_row_range(elements_[rowIdx].cbegin(), elements_[rowIdx].cend());;
	}


	// 返回第colIdx行range对象
	auto col(unsigned colIdx) {
		unsigned rowIdx(0);
		row_range r = row(rowIdx);
		while (r.empty() && rowIdx < rows())
			r = row(++rowIdx);

		auto first = col_element_iter(*this, r, rowIdx, colIdx);
		auto last = col_element_iter(*this, elements_.back().end(), rows(), colIdx);

		return col_range(first, last);
	}

	auto col(unsigned colIdx) const {
		unsigned rowIdx(0);
		const_row_range r = row(rowIdx);
		while (r.empty() && rowIdx < rows())
			r = row(++rowIdx);

		auto first = const_col_element_iter(*this, r, rowIdx, colIdx);
		auto last = const_col_element_iter(*this, elements_.back().end(), rows(), colIdx);

		return const_col_range(first, last);
	}



    // 获取第row行有效数据的数量
    unsigned nonDefaultsOfRow(unsigned row) const {
        return elements_[row].size();
    }


    // 在（row, col）位置插入数据val
	// 若不允许多值，且（row, col）已有数据，则改写该位置数据为val
    void insert(unsigned row, unsigned col, const_reference val) {
		if (!bMultiVal) { // 判定(row, col)位置是否已存在数据
			auto pos = getAt(row, col, false);
			if (pos != elements_[row].end()) { // 已有数据
				pos->second = val;
				return;
			}
		}

        elements_[row].emplace_back(col, val);
    }


    // 将第row行、第col列所有数据置为default
    void setDefault(unsigned row, unsigned col) {
        auto iter = getAt(row, col, false);
		if (iter == elements_[row].end())
			return;

		iter = elements_[row].erase(iter);
		while (iter != elements_[row].end()) {
			if (iter->first == col)
				iter = elements_[row].erase(iter);
			else
				++iter;
		}
    }

    // 将第row行、第col列，且值等于val的数据置为default
	// 返回true，表示找到该元素并置default
	// 返回false表示未在(row, col)位置找到val数据
    bool setDefault(unsigned row, unsigned col, const_reference val) {
		if (!bMultiVal) {
			assert((*this)(row, col) == val);
			return setDefault(row, col);
		}

        auto iter = getAt(row, col, false);
		while (iter != elements_[row].end()) {
			if (iter->first == col && iter->second == val) {
				elements_[row].erase(iter);
				return true;
			}
		}

		return false;
    }


    // 将第row行所有数据置为val
    void assignRow(unsigned r, const_reference val) {
        if(val == default_)
            elements_[r].clear();
        else {
            auto& x = elements_[r];
            x.resize(cols());
            for (auto i = 0u; i < cols(); i++)
                x[i] = { i, val };
        }
    }

    // 将第col列所有数据置为val
    void assignCol(unsigned col, const_reference val) {
        if (val == default_) {
            for (unsigned r = 0; r < rows(); r++)
                setDefault(r, col);
        }
        else {
            for (unsigned r = 0; r < rows(); r++) // TODO: 处理多值情况
                (*this)(r, col) = val;
        }
    }


    // 删除第row行
    void eraseRow(unsigned row) { 
        elements_.erase(std::next(elements_.begin(), row));
    }

    // 删除第col列
    void eraseCol(unsigned col) {
        for (auto& row : elements_) 
            for (auto iter = row.begin(); iter != row.end();) {
                if (iter->first == col) {
                    iter = elements_[row].erase(iter);
                    continue;
                }
                else if (iter->first >= col)
                    iter->first--; // 少了一列，要修正列索引

                ++iter;
            }

        --cols_;
    }


	row_element_iter erase(unsigned row, row_element_iter iter) {
		return elements_[row].erase(iter);
	}


    void appendRow(const_reference val) {
        elements_.push_back(row_type());
        if (val != default_)
            assignRow(rows() - 1, val);
    }

    void appendCol(const_reference val) {
        ++cols_;
        if (val != default_)
            assignCol(cols() - 1, val);
    }


private:

    // 返回第row行、第col列数据读写地址，若不存在且insert非真，则返回end迭代器
    row_element_iter getAt(unsigned row, unsigned col, bool insert) {
        auto& r = elements_[row];

        auto iter = r.begin();
        for (; iter != r.end(); ++iter) {
            if (iter->first == col) // bingo
                return iter;
            
            // 继续找
        }

        return insert ? r.emplace(r.end(), col, default_) : r.end();
    }

private:
    value_type default_;
    unsigned cols_;
    std::vector<row_type> elements_;
};

