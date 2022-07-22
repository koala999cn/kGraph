#pragma once
#include <assert.h>


/* ����"A Filter-based Algorithm for Efficient Composition of Finite-State Transducers"ʵ�ֵ�compositionϵ���˲���.
 *
 * �˲����� = (T1, T2, Q3, i3, ��, ��, ��3)
 *   Q3 -- �˲���״̬��
 *   i3 -- �˲�����ʼ״̬, ��Q3
 *   �� -- �˲���blocking״̬, ��Q3
 *   �� -- ת���˲���E1^L��E2^L��Q3 -> E1'��E2'��Q3. ���У�
 *         E1^L = E1[q]��{(q,��,��^L,1,q)} for each q �� Q1. 
 *         E2^L = E2[q]��{(q,��^L,��,1,q)} for each q �� Q2.
 *   ��3 -- ��̬Ȩֵ�˲���Q3 ->��
 */



/* Implementation of Epsilon-Matching Filter.
 *
 * This filter handles epsilon labels, but disallows redundant epsilon paths,
 * preferring those that match actual �� lebel. It leaves transitions and
 * final weights unmodified.
 *
 * Q3 = { 0, 1, 2, �� }; i3 = 0; ��(q3) = 1, for any q3 �� Q3; ��(e1, e2, q3)=(e1, e2, q3'). ���У�
 *          - 0 if(o[e1], i[e2) = (x, x) with x �� B
 *          - 0 if(o[e1], i[e2) = (��, ��) and q3 = 0
 *    q3' = - 1 if(o[e1], i[e2) = (��^L, ��) and q3 != 2
 *          - 2 if(o[e1], i[e2) = (��, ��^L) and q3 != 1
 *          - �� otherwise
 */

template<typename FST1, typename FST2, typename FST1::alpha_type epsL>
class KtEpsMatchFilter
{
	static_assert(FST1::eps == FST2::eps, "epsilon mismatch for composite matching filter");

public:
	using state_index_t = char;
	using weight_type = typename FST1::weight_type;
	using trans1_type = typename FST1::trans_type;
	using trans2_type = typename FST2::trans_type;
	constexpr static state_index_t blocking_state = -1;
	constexpr static typename FST1::alpha_type epsL = epsL;


	// do the actual filter
	state_index_t operator()(state_index_t q3, const trans1_type& t1, const trans2_type& t2) {
		if (t1.osym == t2.isym) {
			if (t1.osym != FST1::eps && t1.osym != epsL || // (x, x)
				t1.osym == FST1::eps && q3 == 0) // (��, ��) and q3 = 0
				return 0;
		}
		else if (t1.osym == epsL && t2.isym == FST2::eps && q3 != 2) // (��^L, ��) and q3 != 2
			return 1;
		else if (t1.osym == FST1::eps && t2.isym == epsL && q3 != 1) // (��, ��^L) and q3 != 1
			return 2;

		return blocking_state; // otherwise
	}


	// return i3, the initial state of this filter
	constexpr static state_index_t initial() {
		return 0;
	}


	// return weight of state q3
	constexpr static weight_type weight(state_index_t q3) {
		assert(!blocking(q3));
		return weight_type::one();
	}


	// determin if q3 is a blocking state
	constexpr static bool blocking(state_index_t q3) {
		return q3 == blocking_state;
	}
};



/* Implementation of Epsilon-Seqencing Filter.
*
* This filter favors epsilon paths consiting of (output) ��-transitions in T1
* (matched with staying at the same state in T2) followed by (input) 
* ��-transitions in T2(matched with staying at the same state in T1).
*
* Q3 = { 0, 1, �� }, i3 = 0, ��(q3) = 1, ��(e1, e2, q3)=(e1, e2, q3'), ���У�
*          - 0 if(o[e1], i[e2) = (x, x) with x �� B,
*          - 0 if(o[e1], i[e2) = (��, ��^L) and q3 == 0,
*    q3' = - 1 if(o[e1], i[e2) = (��^L, ��),
*          - �� otherwise.
*/

template<typename FST1, typename FST2, typename FST1::alpha_type epsL>
class KtEpsSeqFilter
{
	static_assert(FST1::eps == FST2::eps, "epsilon mismatch for composite matching filter");

public:
	using state_index_t = char;
	using weight_type = typename FST1::weight_type;
	using trans1_type = typename FST1::trans_type;
	using trans2_type = typename FST2::trans_type;
	constexpr static state_index_t blocking_state = -1;
	constexpr static typename FST1::alpha_type epsL = epsL;


	// do the actual filter
	state_index_t operator()(state_index_t q3, const trans1_type& t1, const trans2_type& t2) {

		if (t1.osym == t2.isym && t1.osym != FST1::eps && t1.osym != epsL) // (x, x)
			return 0;
		else if (t1.osym == FST1::eps && t2.isym == epsL && q3 == 0) // (��, ��^L) and q3 == 1
			return 0;		
		else if (t1.osym == epsL && t2.isym == FST2::eps) // (��^L, ��)
			return 1;


		return blocking_state; // otherwise
	}


	// return i3, the initial state of this filter
	constexpr static state_index_t initial() {
		return 0;
	}


	// return weight of state q3
	constexpr static weight_type weight(state_index_t q3) {
		assert(!blocking(q3));
		return weight_type::one();
	}


	// is q3 a blocking state
	constexpr static bool blocking(state_index_t q3) {
		return q3 == blocking_state;
	}
};