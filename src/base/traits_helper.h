#pragma once


// �����ж�ĳ�����Ƿ���й�����xxx��Ա����
#define HAS_MEMBER(xxx) \
    template<typename T, typename... Args> \
    struct has_member_##xxx \
    { \
    private: \
        template<typename U> static auto Check(int) -> decltype(std::declval<U>().xxx(std::declval<Args>()...), std::true_type()); \
        template<typename U> static std::false_type Check(...); \
    public: \
        static constexpr auto value = decltype(Check<T>(0))::value; \
    };


// �����ж�ĳ�����Ƿ���й�����xxx��̬��Ա����
#define HAS_STATIC_MEMBER(xxx) \
    template<typename T, typename... Args> \
    struct has_static_member_##xxx \
    { \
    private: \
        template<typename U> static auto Check(int) -> decltype(U::xxx(std::declval<Args>()...), std::true_type()); \
        template<typename U> static std::false_type Check(...); \
    public: \
        static constexpr auto value = decltype(Check<T>(0))::value; \
    };