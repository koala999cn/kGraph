#pragma once
#include <cstdint>
#include "../base/KtHolder.h"
#include "../base/traits_helper.h"


template<typename VERTEX_TYPE>
struct vertex_traits
{
	using vertex_type = VERTEX_TYPE;


	// �ɾ��еľ�̬��Ա����
	//static auto outdegree(VERTEX_TYPE);
	//static auto indegree(VERTEX_TYPE);

	//static auto outedges(VERTEX_TYPE);
	//static auto inedges(VERTEX_TYPE);

	//static auto weight(VERTEX_TYPE);

	//static auto edgeindex(VERTEX_TYPE); ����flatͼ��Ч
};



template<>
struct vertex_traits<std::int32_t>
{
	using vertex_type = std::int32_t;

	static vertex_type edgeindex(const vertex_type& v) { return v; }
	static vertex_type& edgeindex(vertex_type& v) { return v; }
};

template<>
struct vertex_traits<const std::int32_t>
{
	using vertex_type = const std::int32_t;

	static std::int32_t edgeindex(vertex_type v) { return v; }
};


template<>
struct vertex_traits<std::uint32_t>
{
	using vertex_type = std::uint32_t;

	static vertex_type edgeindex(const vertex_type& v) { return v; }
	static vertex_type& edgeindex(vertex_type& v) { return v; }
};

template<>
struct vertex_traits<const std::uint32_t>
{
	using vertex_type = const std::uint32_t;

	static std::uint32_t edgeindex(vertex_type v) { return v; }
};


template<>
struct vertex_traits<std::int64_t>
{
	using vertex_type = std::int64_t;

	static vertex_type edgeindex(const vertex_type& v) { return v; }
	static vertex_type& edgeindex(vertex_type& v) { return v; }
};

template<>
struct vertex_traits<const std::int64_t>
{
	using vertex_type = const std::int64_t;

	static std::int64_t edgeindex(vertex_type v) { return v; }
};


template<>
struct vertex_traits<std::uint64_t>
{
	using vertex_type = std::uint64_t;

	static vertex_type edgeindex(const vertex_type& v) { return v; }
	static vertex_type& edgeindex(vertex_type& v) { return v; }
};

template<>
struct vertex_traits<const std::uint64_t>
{
	using vertex_type = const std::uint64_t;

	static std::uint64_t edgeindex(vertex_type v) { return v; }
};


namespace kPrivate
{
	template<typename VERTEX_TYPE>
	class KtFlatVertexWrapper_ : public KtHolder<VERTEX_TYPE>
	{
	public:
		using super_ = KtHolder<VERTEX_TYPE>;

		unsigned edgeindex() const { return edgeindex_; }
		unsigned& edgeindex() { return edgeindex_; }

	private:
		unsigned edgeindex_;
	};

	template<>
	struct KtFlatVertexWrapper_<void> : public KtHolder<unsigned>
	{
	public:
		decltype(auto) edgeindex() const { return inside(); }
		decltype(auto) edgeindex() { return inside(); }
	};
};


template<typename VERTEX_TYPE>
struct vertex_traits<kPrivate::KtFlatVertexWrapper_<VERTEX_TYPE>>
	: public vertex_traits<VERTEX_TYPE>
{
	using vertex_type = kPrivate::KtFlatVertexWrapper_<VERTEX_TYPE>;

	static decltype(auto) edgeindex(const vertex_type& v) { return v.edgeindex(); }
	static decltype(auto) edgeindex(vertex_type& v) { return v.edgeindex(); }
};


template<typename VERTEX_TYPE>
struct vertex_traits<const kPrivate::KtFlatVertexWrapper_<VERTEX_TYPE>>
	: public vertex_traits<VERTEX_TYPE>
{
	using vertex_type = const kPrivate::KtFlatVertexWrapper_<VERTEX_TYPE>;

	static decltype(auto) edgeindex(vertex_type& v) { return v.edgeindex(); }
};


namespace kPrivate
{
	HAS_STATIC_MEMBER(edgeindex);
	HAS_STATIC_MEMBER(outdegree);
	HAS_STATIC_MEMBER(outedges);
	HAS_STATIC_MEMBER(inedges);
}


// ����VERTEX_TRAITS�����й�bool����
template<typename VERTEX_TRAITS>
struct vertex_traits_helper
{
	using vertex_type = typename VERTEX_TRAITS::vertex_type;

	constexpr static bool has_edgeindex = kPrivate::has_static_member_edgeindex<VERTEX_TRAITS, vertex_type>::value;
	constexpr static bool has_outdegree = kPrivate::has_static_member_outdegree<VERTEX_TRAITS, vertex_type>::value;
	constexpr static bool has_outedges = kPrivate::has_static_member_outedges<VERTEX_TRAITS, vertex_type>::value;
	constexpr static bool has_inedges = kPrivate::has_static_member_inedges<VERTEX_TRAITS, vertex_type>::value;
};


namespace kPrivate
{
	template<typename VERTEX_TYPE>
	constexpr static bool hasEdgeIndex_() {
		return vertex_traits_helper<vertex_traits<VERTEX_TYPE>>::has_edgeindex;
	}

	template<typename VERTEX_TYPE>
	constexpr static bool hasOutDegree_() {
		return vertex_traits_helper<vertex_traits<VERTEX_TYPE>>::has_outdegree;
	}
}


template<typename VERTEX_TYPE>
inline constexpr bool has_edgeindex_v = kPrivate::hasEdgeIndex_<VERTEX_TYPE>();

template<typename VERTEX_TYPE>
inline constexpr bool has_outdegree_v = kPrivate::hasOutDegree_<VERTEX_TYPE>();


// ��VERTEX_TYPE������ƫ����Ϣ��ʹ��KtFlatVertexWrapper_����һ��
template<typename VERTEX_TYPE>
using vertex_has_edgeindex_t = std::conditional_t<std::is_void_v<VERTEX_TYPE>, unsigned,
	std::conditional_t<has_edgeindex_v<VERTEX_TYPE>,
	VERTEX_TYPE, kPrivate::KtFlatVertexWrapper_<VERTEX_TYPE>>>;