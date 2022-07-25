#pragma once
#include "KuEndian.h"


class KmEndianed
{
public:
	KmEndianed() {
		big_ = KuEndian::isNativeBig();
	}

	// �趨���ȡĿ�����ݵĴ�С�˸�ʽ
	void setBigEndian(bool big = true) { big_ = big; }
	void setLittleEndian(bool little = true) { big_ = !little; }
                void flipEndian() { big_ = !big_; }
	bool isBigEndian() const { return big_; }
	bool isLittleEndian() const { return !big_; }


	// ƥ��Ŀ�������뱾�����ݴ�С�˸�ʽ
	template<typename T>
	void matchEndian(T& val) {
		if (KuEndian::isNativeBig() != big_)
			KuEndian::swap(val);
	}


	// ����ƥ��
	template<typename T>
	void matchEndian(T buf[], unsigned count) {
		if (KuEndian::isNativeBig() != big_)
			KuEndian::swap(buf, count);
	}

private:
	bool big_; // Ŀ�������Ƿ��˸�ʽ��
};

