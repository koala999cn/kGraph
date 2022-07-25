#pragma once


class KuEndian
{
public:
    static bool isNativeLittle();
    static bool isNativeBig();


	// 对类型T的数据进行大小端转换
	template<typename T>
	static void swap(T& val) {
		if (sizeof(T) == 2)
			swapBytes2(reinterpret_cast<char*>(&val));
		else if (sizeof(T) == 4)
			swapBytes4(reinterpret_cast<char*>(&val));
		else if (sizeof(T) == 8)
			swapBytes8(reinterpret_cast<char*>(&val));
		else if (sizeof(T) == 16)
			swapBytes16(reinterpret_cast<char*>(&val));
		else if (sizeof(T) > 1)
			swapBytes(reinterpret_cast<char*>(&val), sizeof(T));
	}


	// 对count个T元素作大小端转换
	template<typename T>
	static void swap(T* buf, unsigned count) {
		if (sizeof(T) == 2)
			for (unsigned i = 0; i < count; i++)
				swapBytes2(reinterpret_cast<char*>(buf++));
		else if (sizeof(T) == 4)
			for (unsigned i = 0; i < count; i++)
				swapBytes4(reinterpret_cast<char*>(buf++));
		else if (sizeof(T) == 8)
			for (unsigned i = 0; i < count; i++)
				swapBytes8(reinterpret_cast<char*>(buf++));
		else if (sizeof(T) == 16)
			for (unsigned i = 0; i < count; i++)
				swapBytes16(reinterpret_cast<char*>(buf++));
		else if (sizeof(T) > 1)
			for (unsigned i = 0; i < count; i++)
				swapBytes(reinterpret_cast<char*>(buf++), sizeof(T));
	}


	// 小端数据val转换为本地数据
	template<typename T>
	static void fromLittle(T& val) {
		if (isNativeBig()) swap(val);
	}

	// 本地数据val转换为小端数据
	template<typename T>
	static void toLittle(T& val) {
		fromLittle(val);
	}


	// 大端数据val转换为本地数据
	template<typename T>
	static void fromBig(T& val) {
		if (isNativeLittle()) swap(val);
	}

	// 本地数据val转换为大端数据
	template<typename T>
	static void toBig(T& val) {
		fromBig(val);
	}


	// 小端数组buf批量转换为本地数据
	template<typename T>
	static void fromLittle(T* buf, unsigned count) {
		if (isNativeBig()) swap(buf, count);
	}

	// 本地数据批量转换为小端数据
	template<typename T>
	static void toLittle(T* buf, unsigned count) {
		fromLittle(buf, count);
	}


	// 大端数组buf批量转换为本地数据
	template<typename T>
	static void fromBig(T* buf, unsigned count) {
		if (isNativeLittle()) swap(buf, count);
	}

	// 本地数据批量转换为大端数据
	template<typename T>
	static void toBig(T* buf, unsigned count) {
		fromBig(buf, count);
	}


	// 对2字节的数据in进行大小端转换
    static void swapBytes2(char* in);

	// 对4字节的数据in进行大小端转换
    static void swapBytes4(char* in);

	// 对8字节的数据in进行大小端转换
    static void swapBytes8(char* in);

	// 对16字节的数据in进行大小端转换
    static void swapBytes16(char* in);

	// 对size字节的数据in进行大小端转换
	static void swapBytes(char* in, unsigned size);


private:
    KuEndian() = delete;
};
