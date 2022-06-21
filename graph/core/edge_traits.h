#pragma once
#include "../base/KtHolder2.h"
#include "../base/traits_helper.h"


// default traits
template<typename EDGE_TYPE>
struct edge_traits
{
	using edge_type = EDGE_TYPE;
	using weight_type = EDGE_TYPE;

	static weight_type weight(const edge_type& edge) { return edge; }

	template<typename T = edge_type, typename = std::enable_if_t<!std::is_const_v<T>>>
	static weight_type& weight(edge_type& edge) { return edge; }

	inline static const edge_type null_edge = {};
};


namespace kPrivate
{
	template<typename EDGE_TYPE>
	using KtEdgeWrapper_ = KtHolder2<unsigned, EDGE_TYPE>;

	HAS_STATIC_MEMBER(to)
};


template<typename EDGE_TYPE>
struct edge_traits<kPrivate::KtEdgeWrapper_<EDGE_TYPE>>
	: public edge_traits<EDGE_TYPE>
{
	using edge_type = kPrivate::KtEdgeWrapper_<EDGE_TYPE>;

	static_assert(!std::is_trivial_v<EDGE_TYPE> || std::is_trivial_v <edge_type>,
		"constructing trivial edge wrapper error");

	static decltype(auto) to(const edge_type& wrap_edge) { return wrap_edge.outside(); }
	static decltype(auto) to(edge_type& wrap_edge) { return wrap_edge.outside(); }
};


template<typename EDGE_TYPE>
struct edge_traits<const kPrivate::KtEdgeWrapper_<EDGE_TYPE>>
	: public edge_traits<const EDGE_TYPE>
{
	using edge_type = const kPrivate::KtEdgeWrapper_<EDGE_TYPE>;

	static decltype(auto) to(edge_type& wrap_edge) { return wrap_edge.outside(); }
};


// 根据EDGE_TRAITS生成有关bool常量
template<typename EDGE_TRAITS>
struct edge_traits_helper
{
	using edge_type = typename EDGE_TRAITS::edge_type;

	constexpr static bool has_to = kPrivate::has_static_member_to<EDGE_TRAITS, edge_type>::value;
};


namespace kPrivate
{
	template<typename EDGE_TYPE>
	constexpr static bool hasTo_() {
		return edge_traits_helper<edge_traits<EDGE_TYPE>>::has_to;
	}
}


template<typename EDGE_TYPE>
inline constexpr bool has_to_v = kPrivate::hasTo_<EDGE_TYPE>();


// 若EDGE_TYPE不带to信息，使用KtEdgeWrapper_附加一个
template<typename EDGE_TYPE>
using edge_has_to_t = std::conditional_t<has_to_v<EDGE_TYPE>,
	EDGE_TYPE, kPrivate::KtEdgeWrapper_<EDGE_TYPE>>;