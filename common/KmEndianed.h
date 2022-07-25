#pragma once
#include "KuEndian.h"


class KmEndianed
{
public:
	KmEndianed() {
		big_ = KuEndian::isNativeBig();
	}

	// 设定与获取目标数据的大小端格式
	void setBigEndian(bool big = true) { big_ = big; }
	void setLittleEndian(bool little = true) { big_ = !little; }
                void flipEndian() { big_ = !big_; }
	bool isBigEndian() const { return big_; }
	bool isLittleEndian() const { return !big_; }


	// 匹配目标数据与本地数据大小端格式
	template<typename T>
	void matchEndian(T& val) {
		if (KuEndian::isNativeBig() != big_)
			KuEndian::swap(val);
	}


	// 批量匹配
	template<typename T>
	void matchEndian(T buf[], unsigned count) {
		if (KuEndian::isNativeBig() != big_)
			KuEndian::swap(buf, count);
	}

private:
	bool big_; // 目标数据是否大端格式？
};

