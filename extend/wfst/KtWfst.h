#pragma once
#include <unordered_map>
#include <map>
#include <assert.h>
#include "../graph/GraphX.h"
#include "../graph/core/KtBfsIter.h"
#include "../graph/core/KtAdjIter.h"
#include "../graph/core/vertex_traits.h"
#include "trans_traits.h"


// ���������ʵ���㷨�ο�Mehryar Mohri��ϵ������

// WFST���ڰ뻷(semiring)������Ҫ��8Ҫ�ع��ɣ�
//   �� -- ������ĸ��
//   �� -- �����ĸ��
//   �� -- ״̬��
//   �� -- ��ʼ״̬���������ڣ�
//   �� -- ��ֹ״̬���������ڣ�
//   �� -- ת�Ƽ��������ڣѡ�(�ơ�{��})��(�� ��{��})��������
//   �� -- ��ʼ��Ȩ��������->��
//   �� -- ��ֹ��Ȩ��������->��

// weights are assumed to be cumulated by multiplication along each path and 
// addition over different paths.

/*
  A WFSA is simply a WFST where the output labels have been omitted.
  Similarly, FSAs and FSTs lack weights on the arcs or states.
*/


/*
 * Baisc operations:
 *  - composition
 *  - determinization
 *  - minimization
 *  - epsilon-removal
 *  - epsilon-normalization
 *  - synchronization
 *  - weight-pushing
 *  - reversal
 *  - projection
 *  - shortest-path
 *  - connection
 *  - closure
 *  - concatenation
 *  - pruning
 *  - re-weighting
 *  - union
 */


template<typename TRANS_TYPE, 
	typename TRANS_TRAITS = trans_traits<TRANS_TYPE>, 
	typename GRAPH_TYPE = DigraphPx<TRANS_TYPE>>
class KtWfst : public GRAPH_TYPE
{
public:
	using super_ = GRAPH_TYPE;
	using graph_type = GRAPH_TYPE;
	using trans_type = TRANS_TYPE; // ת������
	using traits_type = TRANS_TRAITS;
	using weight_type = std::decay_t<decltype(traits_type::weight(std::declval<trans_type>()))>; // �뻷���൱��Ȩ��
	using alpha_type = std::decay_t<decltype(traits_type::isym(std::declval<trans_type>()))>; // ��ĸ����
	using typename super_::vertex_index_t;
	using typename super_::vertex_type;
	using typename super_::edge_type;


	// ����wfst��������
	using state_index_t = vertex_index_t;
	using trans_iter = typename graph_type::edge_iter;
	using const_trans_iter = typename graph_type::const_edge_iter;
	using initial_set_type = std::unordered_map<state_index_t, weight_type>; // ��ʼ״̬���ϵ�����
	using final_set_type = std::conditional_t<has_weight_v<vertex_type>, void*, initial_set_type>; // ��ֹ״̬���ϵ�����
	

	constexpr static const auto eps = traits_type::eps;
	constexpr static state_index_t null_state = -1;

	using super_::super_;
	using super_::order;

	// ״̬�Ƿ����Ȩֵ���ԣ�
	constexpr static bool hasStateWeight() {
		return has_weight_v<vertex_type>;
	}

	auto transIter() { return super_::edgeIter(); }
	auto transIter() const { return super_::edgeIter(); }


	state_index_t addState() { return super_::addVertex(); }

	void eraseState(state_index_t s) {
		super_::eraseVertex(s);

		// ���³�ʼ/��ֹ״̬���
		updateStateIdx_(initials_, s);

		if constexpr (hasStateWeight())
		    updateStateIdx_(finals_, s);
	}


	// ����1��ת��
	void addTrans(state_index_t s1, state_index_t s2,
		const alpha_type& isym, const alpha_type& osym, const weight_type& wt, bool merge = false) {

		if (merge) {
			// �����Ƿ����<isym, osym>��ת�ƣ�����������Ҫ�ϲ�����������
			auto iter = adjIter(s1);
			for (; !iter.isEnd(); ++iter) {
				if (*iter == s2) {
					auto& trans = iter.edge();
					if (traits_type::isym(trans) == isym && traits_type::osym(trans) == osym) { // �ϲ�
						iter.reedge(traits_type::construct(isym,  osym, traits_type::weight(trans) + wt));
						return;
					}
				}
			}
		}

		// ����
		addEdge(s1, s2, traits_type::construct(isym, osym, wt));
	}

	void addTrans(state_index_t s1, state_index_t s2, const trans_type& trans) {
		addEdge(s1, s2, trans);
	}


	// ɾ��s1��s2��ת��
	void eraseTrans(state_index_t s1, state_index_t s2) {
		eraseEdge(s1, s2);
	}


	bool hasTrans(state_index_t s1, state_index_t s2) const {
		return hasEdge(s1, s2);
	}

	bool hasTrans(state_index_t s1, state_index_t s2, const trans_type& trans) const {
		return hasEdge(s1, s2, trans);
	}


	auto numInitials() const { return initials_.size(); }

	// ���س�ʼ״̬��
	const initial_set_type& initials() const { return initials_; }

	// �Ƿ����ó�ʼ״̬
	bool hasInitial() const { return !initials_.empty(); }


	// ״̬i�Ƿ�Ϊ��ʼ״̬
	bool isInitial(state_index_t i) const { 
		return initials_.find(i) != initials_.end(); 
	}

	// ���س�ʼ״̬i��Ȩֵ���÷�������i�ض�Ϊ��ʼ״̬
	const weight_type& initialWeight(state_index_t i) const {
		assert(isInitial(i));
		return initials_.at(i);
	}


	// ��������״̬iΪ��ʼ״̬������ĳ�ʼ״̬i��ȨֵΪwt
	void setInitial(state_index_t i, const weight_type& wt = weight_type::one()) {
		assert(i < order());
		initials_.insert_or_assign(i, wt);
	}

	void unsetInitial(state_index_t i) {
		assert(isInitial(i));
		initials_.erase(i);
	}

	// ����һ����״̬����������Ϊ��ʼ״̬��ȨֵΪwt��
	state_index_t addInitial(const weight_type& wt = weight_type::one()) {
		state_index_t i = addState();
		initials_.insert({ i, wt });
		return i;
	}

	// ���س�����ʼ״̬����ţ���ʧ���򷵻�null_state
	state_index_t makeSuperInitial() {
		if (!hasInitial())
			return null_state;
		else if (initials().size() == 1)
			return initials().begin()->first;
		else {
			auto idx = addState();
			for (auto& i : initials())
				addTrans(idx, i.first, eps, eps, i.second);

			initials_.clear();
			setInitial(idx);

			return idx;
		}
	}

	// ������ֹ״̬��
	decltype(auto) finals() const { 
		if constexpr (hasStateWeight()) {
			std::vector<std::pair<state_index_t, weight_type>> fs;
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i))
					fs.push_back({ i, finalWeight(i) });
			return fs;
		}
		else
			return finals_;
	}

	template<typename T = vertex_type, typename = std::enable_if_t<has_weight_v<T>>>
	decltype(auto) stateWeight(state_index_t f) const {
		return vertex_traits<T>::weight(vertexAt(f));
	}

	template<typename T = vertex_type, typename = std::enable_if_t<has_weight_v<T>>>
	decltype(auto) stateWeight(state_index_t f) {
		return vertex_traits<T>::weight(vertexAt(f));
	}

	// ״̬f�Ƿ�Ϊ��ֹ״̬
	bool isFinal(state_index_t f) const {
		if constexpr (hasStateWeight()) {
			if constexpr (std::is_scalar_v<std::decay_t<decltype(stateWeight(f))>>)
			    return weight_type::zero().value() != stateWeight(f);
			else
				return weight_type::zero() != stateWeight(f);
		}
		else
			return finals_.find(f) != finals_.end();
	}

	auto numFinals() const {
		if constexpr (hasStateWeight()) {
			state_index_t num(0);
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i)) num++;
			return num;
		}
		else
			return finals_.size();
	}


	// �Ƿ�������ֹ״̬
	bool hasFinal() const { 
		if constexpr (hasStateWeight()) {
			state_index_t num(0);
			for (state_index_t i = 0; i < order(); i++)
				if (isFinal(i))
					return true;
			return false;
		}
		else
			return !finals_.empty();
	}

	
	// ������ֹ״̬f��Ȩֵ���÷�������f�ض�Ϊ��ֹ״̬
	decltype(auto) finalWeight(state_index_t f) const {
		assert(isFinal(f));
		if constexpr (hasStateWeight())
			return stateWeight(f);
		else
			return finals_.at(f);
	}


	// ��������״̬fΪ��ֹ״̬���������ֹ״̬f��ȨֵΪwt
	void setFinal(state_index_t f, const weight_type& wt = weight_type::one()) {
		assert(f < order());
		if constexpr (hasStateWeight())
			stateWeight(f) = wt;
		else
			finals_.insert_or_assign(f, wt);
	}

	void unsetFinal(state_index_t f) {
		assert(isFinal(f));
		if constexpr (hasStateWeight())
			stateWeight(f) = weight_type::zero();
		else
			finals_.erase(f);
	}

	// ����һ����״̬����������Ϊ��ֹ״̬��ȨֵΪwt��
	state_index_t addFinal(const weight_type& wt = weight_type::one()) {
		state_index_t f = addState();
		if constexpr (hasStateWeight())
			stateWeight(f) = wt;
		else 
			finals_.insert({ f, wt });
		return f;
	}

	/*
	// ���س�����ֹ״̬����ţ���ʧ���򷵻�null_state
	state_index_t makeSuperFinal() {
		if (!hasFinal())
			return null_state;
		else if (finals().size() == 1)
			return finals().begin()->first;
		else {
			auto idx = addState();
			for (auto& f : finals())
				addTrans(f.first, idx, eps, eps, f.second);

			finals_.clear();
			setFinal(idx);

			return idx;
		}
	}*/


	/*
    // �����бߣ�״̬ת�ƣ���ת��������ÿ��ת�Ƶ�����״̬�����״̬��ͬʱ������ʼ״̬����ֹ״̬��
	KtWfst reverse() const {
		KtWfst rwfst;
		rwfst.inside() = inverse<graph_type>();
		rwfst.initials_ = finals_;
		rwfst.finals_ = initials_;
		return rwfst;
	}


	// �����������ת
	KtWfst invert() const {
		struct Weightor_ {
			auto operator()(const trans_type& trans) {
				trans_traits traits;
				return traits.construct(traits.osym(trans), traits.isym(trans), traits.weight(trans)); // ��ת
			}
		};

		KtWfst rwfst;
		rwfst.inside() = copy<graph_type, Weightor_>();
		rwfst.initials_ = initials_;
		rwfst.finals_ = finals_;
	}*/

	/// �����Ż��㷨


	// ɾ�����ڳ�ʼ״̬����ֹ״̬·���ϵĶ���
	void eraseDeadStates() {
		std::vector<bool> dead(order(), true);

		// TODO: һ���򵥵�ʵ�֣����걸
		for (auto& i : initials()) {
			auto bfs = KtBfsIter(*this, i.first);
			for (; !bfs.isEnd(); ++bfs)
				dead[*bfs] = false;
		}

		for (state_index_t s = order() - 1; s != -1; s--) // �������ɾ�����Ա㱣��ͼ�Ķ�����˳��
			if (dead[s]) eraseState(s);
	}


	// �ϲ��ر�������/�����ͬ��ת��
	void mergeSameTrans() {

		for (state_index_t s = 0; s < order(); s++) {

			struct KpTrans {
				state_index_t to;
				alpha_type ilabel;
				alpha_type olabel;

				bool operator < (const KpTrans& other) const {
					return to < other.to || (to == other.to && ilabel < other.ilabel) || 
						(to == other.to && ilabel == other.ilabel && olabel < other.olabel);
				}
			};

			std::map<KpTrans, super_::adj_iter> trans;
			auto iter = adjIter(s);	
			while (!iter.isEnd()) {
				auto& edge = iter.edge();
				KpTrans t{ *iter, traits_type::isym(edge), traits_type::osym(edge) };
				auto miter = trans.find(t);
				if (miter != trans.end()) { // ��Ҫ�ϲ�
					miter->second.reedge(traits_type::construct(traits_type::isym(edge), traits_type::osym(edge),
						traits_type::weight(edge) + traits_type::weight(miter->second.edge())));
					iter.erase();
				}
				else {
					trans.insert({ t, iter });
					++iter;
				}
			}
		}
	}


private:

	// ɾ��states�м�ֵΪs��Ԫ�أ�������states�����д���s�ļ�ֵΪs-1
	static void updateStateIdx_(initial_set_type& states, state_index_t s) {
		initial_set_type newStates;
		newStates.reserve(states.size());
		for (auto& f : states) {
			if (f.first < s)
				newStates.insert({ f.first, f.second });
			else if (f.first > s)
				newStates.insert({ f.first - 1, f.second });
		}
		std::swap(states, newStates);
	}

private:
	initial_set_type initials_; // ��ʼ״̬������Ȩֵ
	final_set_type finals_; // ��ֹ״̬������Ȩֵ
};
