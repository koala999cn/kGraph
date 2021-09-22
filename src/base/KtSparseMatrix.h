#pragma once
#include <vector>
#include <algorithm>


// 基于vector的稀疏矩阵实现
template<typename T, typename ROW_CONTAINER = std::vector<std::pair<unsigned, T>>>
class KtSparseMatrix
{
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using element_type = std::pair<unsigned, T>;
    using row_type = ROW_CONTAINER;
    using element_iterator =  typename row_type::iterator;
    using const_element_iterator = typename row_type::const_iterator;

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
        auto iter = const_cast<KtSparseMatrix<value_type>*>(this)->getAt(row, col, false);
        return iter == rowEnd(row) ? default_ : iter->second;
    }


    // 等同于assign(row, col, val)
    reference operator()(unsigned row, unsigned col) {
        auto iter = getAt(row, col, true);
        assert(iter != rowEnd(row));
        return iter->second;
    }

    unsigned rows() const { return elements_.size(); }
    unsigned cols() const { return cols_; }

    
    // 返回第row行对象
    row_type& row(unsigned row) { return elements_[row]; }
    const row_type& row(unsigned row) const { return elements_[row]; }


    // 获取第row行有效数据的数量
    unsigned nonDefaultsOfRow(unsigned row) const {
        return elements_[row].size();
    }


    // 直接在（row, col）插入元素val，不检测元素是否已存在，以支持同一位置多个值
    void insert(unsigned row, unsigned col, const_reference val) {
        elements_[row].emplace_back(col, val);
    }


    auto rowBegin(unsigned row) const {
        return elements_[row].begin();
    }

    auto rowEnd(unsigned row) const {
        return elements_[row].end();
    }


    // 将第row行、第col列数据置为default
    // 在允许多值情况下，仅设置第一个值为default
    void setDefault(unsigned row, unsigned col) {
        auto iter = getAt(row, col, false);
        if (iter != elements_[row].end())
            elements_[row].erase(iter);
    }

    // 将第row行、第col列的所有数据均置为default
    void defaultAll(unsigned row, unsigned col) {
        auto iter = getAt(row, col, false);
        elements_[row].erase(iter);

        while (iter != elements_[row].end()) {
            if (iter->first == col)
                elements_[row].erase(iter);
            else
                ++iter;
        }
    }

    // 将第row行所有数据置为default
    void assignRow(unsigned r, const_reference val) {
        if(val == default_)
            elements_[r].clear();
        else {
            auto& x = row(r);
            x.resize(cols());
            for (auto i = 0u; i < cols(); i++)
                x[i] = { i, val };
        }
    }

    // 将第col列所有数据置为default
    void assignCol(unsigned col, const_reference val) {
        if (val == default_) {
            for (unsigned r = 0; r < rows(); r++)
                defaultAll(r, col);
        }
        else {
            for (unsigned r = 0; r < rows(); r++)
                (*this)(r, col) = val;
        }
    }


    // 删除第row行
    void eraseRow(unsigned row) { 
        elements_.erase(elements_.begin() + row);
    }

    // 删除第col列
    void eraseCol(unsigned col) {
        for (auto& row : elements_) 
            for (auto iter = row.begin(); iter != row.end();) {
                if (iter->first == col) {
                    elements_[row].erase(iter);
                    continue;
                }
                else if (iter->first >= col)
                    iter->first--; // 少了一列，要修正列索引

                ++iter;
            }

        --cols_;
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

    // 返回第row行、第col列数据读写地址，若不存在且insert非真，则返回null
    element_iterator getAt(unsigned row, unsigned col, bool insert) {
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

