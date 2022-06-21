#pragma once
#include <assert.h>


/* ¸ù¾Ý"A Filter-based Algorithm for Efficient Composition of Finite-State Transducers"ÊµÏÖµÄcompositionÏµÁÐÂË²¨Æ÷.
 *
 * ÂË²¨Æ÷¦µ = (T1, T2, Q3, i3, ¡Í, ¦×, ¦Ñ3)
 *   Q3 -- ÂË²¨Æ÷×´Ì¬¼¯
 *   i3 -- ÂË²¨Æ÷³õÊ¼×´Ì¬, ¡ÊQ3
 *   ¡Í -- ÂË²¨Æ÷blocking×´Ì¬, ¡ÊQ3
 *   ¦× -- ×ªÒÆÂË²¨£ºE1^L¡ÁE2^L¡ÁQ3 -> E1'¡ÁE2'¡ÁQ3. ÆäÖÐ£º
 *         E1^L = E1[q]¡È{(q,¦Å,¦Å^L,1,q)} for each q ¡Ê Q1. 
 *         E2^L = E2[q]¡È{(q,¦Å^L,¦Å,1,q)} for each q ¡Ê Q2.
 *   ¦Ñ3 -- ÖÕÌ¬È¨ÖµÂË²¨£ºQ3 ->¦ª
 */



/* Implementation of Epsilon-Matching Filter.
 *
 * This filter handles epsilon labels, but disallows redundant epsilon paths,
 * preferring those that match actual ¦Å lebel. It leaves transitions and
 * final weights unmodified.
 *
 * Q3 = { 0, 1, 2, ¡Í }; i3 = 0; ¦Ñ(q3) = 1, for any q3 ¡Ê Q3; ¦×(e1, e2, q3)=(e1, e2, q3'). ÆäÖÐ£º
 *          - 0 if(o[e1], i[e2) = (x, x) with x ¡Ê B
 *          - 0 if(o[e1], i[e2) = (¦Å, ¦Å) and q3 = 0
 *    q3' = - 1 if(o[e1], i[e2) = (¦Å^L, ¦Å) and q3 != 2
 *          - 2 if(o[e1], i[e2) = (¦Å, ¦Å^L) and q3 != 1
 *          - ¡Í otherwise
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
				t1.osym == FST1::eps && q3 == 0) // (¦Å, ¦Å) and q3 = 0
				return 0;
		}
		else if (t1.osym == epsL && t2.isym == FST2::eps && q3 != 2) // (¦Å^L, ¦Å) and q3 != 2
			return 1;
		else if (t1.osym == FST1::eps && t2.isym == epsL && q3 != 1) // (¦Å, ¦Å^L) and q3 != 1
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
* This filter favors epsilon paths consiting of (output) ¦Å-transitions in T1
* (matched with staying at the same state in T2) followed by (input) 
* ¦Å-transitions in T2(matched with staying at the same state in T1).
*
* Q3 = { 0, 1, ¡Í }, i3 = 0, ¦Ñ(q3) = 1, ¦×(e1, e2, q3)=(e1, e2, q3'), ÆäÖÐ£º
*          - 0 if(o[e1], i[e2) = (x, x) with x ¡Ê B,
*          - 0 if(o[e1], i[e2) = (¦Å, ¦Å^L) and q3 == 0,
*    q3' = - 1 if(o[e1], i[e2) = (¦Å^L, ¦Å),
*          - ¡Í otherwise.
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
		else if (t1.osym == FST1::eps && t2.isym == epsL && q3 == 0) // (¦Å, ¦Å^L) and q3 == 1
			return 0;		
		else if (t1.osym == epsL && t2.isym == FST2::eps) // (¦Å^L, ¦Å)
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