#include "KuEndian.h"
#include <algorithm>


namespace kPrivate
{
    enum class KeEndian { k_big, k_little };

    static KeEndian getNativeEndian()
    {
		// TODO: use std::call_once
        union
        {
            char big_endian_1[2];
            short is_it_really_1;
        } u;

        u.big_endian_1[0] = 0;
        u.big_endian_1[1] = 1;

        return (u.is_it_really_1 == 1) ? KeEndian::k_big : KeEndian::k_little;
    }
}


bool KuEndian::isNativeLittle()
{
	using namespace kPrivate;
    return getNativeEndian() == KeEndian::k_little;
}


bool KuEndian::isNativeBig()
{
	using namespace kPrivate;
    return getNativeEndian() == KeEndian::k_big;
}


void KuEndian::swapBytes(char* in, unsigned size)
{
    char* start = in;
    char* end = start+size-1;
    while(start < end)
        std::swap(*start++, *end--);
}


void KuEndian::swapBytes2(char* in)
{
    std::swap(in[0], in[1]);
}


void KuEndian::swapBytes4(char* in)
{
    std::swap(in[0], in[3]);
    std::swap(in[1], in[2]);
}


void KuEndian::swapBytes8(char* in)
{
    std::swap(in[0], in[7]);
    std::swap(in[1], in[6]);
    std::swap(in[2], in[5]);
    std::swap(in[3], in[4]);
}


void KuEndian::swapBytes16(char* in)
{
    std::swap(in[0], in[15]);
    std::swap(in[1], in[14]);
    std::swap(in[2], in[13]);
    std::swap(in[3], in[12]);
    std::swap(in[4], in[11]);
    std::swap(in[5], in[10]);
    std::swap(in[6], in[9]);
    std::swap(in[7], in[8]);
}
