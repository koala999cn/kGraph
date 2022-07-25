#pragma once
#include "istreamx.h"
#include "stlex.h"
#include <vector>
#include <string>
#include <assert.h>

class KgModelSet;
class KcGmm;

class KuBasicIO
{
public:

	constexpr static std::int32_t kNoPdf = -1;

	// return true when see binary flags
	static bool binaryTest(std::istream& strm);

	template<typename T>
	static bool readBasicType(stdx::istreamx &strm, T& val);

	// ��ȡ��������
	template<typename T>
	static bool readIntVector(stdx::istreamx& strm, std::vector<T>& vec);

	// ��ȡ�������У���Ӧkaldi��Vector�ṹ�����б�Ҫ��������ת��
	template<typename T>
	static bool readFloatVector(stdx::istreamx &strm, std::vector<T>& vec);

	// ��ȡ������󣬶�Ӧkaldi��Matrix�ṹ�����б�Ҫ��������ת��
	template<typename T>
	static bool readFloatMatrix(stdx::istreamx& strm, std::vector<std::vector<T>>& mat);

	template<typename T>
	static bool readTable(stdx::istreamx& strm, std::string& key, std::vector<std::vector<T>>& table);


private:
	KuBasicIO() = delete;
};


template<typename T>
bool KuBasicIO::readBasicType(stdx::istreamx &strm, T& val)
{
	if (strm.binary()) {
		int tsize = strm->get();
		if (tsize != sizeof(T))
			return false;

		strm >> val;
	}
	else {
		if (sizeof(T) == 1) {
			std::int16_t tmp;
			strm >> tmp;
			val = static_cast<T>(tmp);
		}
		else
			strm >> val;
	}

	return strm;
}


template<typename T>
bool KuBasicIO::readIntVector(stdx::istreamx& strm, std::vector<T>& vec)
{
	static_assert(std::is_integral_v<T>, "T must be integral type");
	int tsize = strm->get();
	return tsize == sizeof(T) && strm >> vec;
}


template<typename T>
bool KuBasicIO::readFloatVector(stdx::istreamx &strm, std::vector<T>& vec)
{
	static_assert(std::is_floating_point_v<T>, "T must be floating type");

	if (!strm.binary()) // ֻʵ��binaryģʽ�µĶ�ȡ
		return false;

	std::string sig;
	strm >> sig;
	if (sig != "DV" && sig != "FV")
		return false;

	std::int32_t vecSize;
	if (!readBasicType(strm, vecSize))
		return false;

	vec.resize(vecSize);
	if (sig[0] == 'D' && sizeof(T) == sizeof(double) ||
		sig[0] == 'F' && sizeof(T) == sizeof(float))
		return strm.read(&vec[0], vecSize); // ����һ�£�ֱ�Ӷ�ȡ

	// ��Ҫ���и�������ת��
	if (sig[0] == 'D') { // doubleת��Ϊfloat��long double
		std::vector<double> tmp(vecSize);
		if (!strm.read(&tmp[0], vecSize))
			return false;
		stdx::copy(tmp, vec);
	}
	else { // floatת��Ϊdouble��long double
		std::vector<float> tmp(vecSize);
		if (!strm.read(&tmp[0], vecSize))
			return false;
		stdx::copy(tmp, vec);
	}

	return true;
}


template<typename T>
bool KuBasicIO::readFloatMatrix(stdx::istreamx& strm, std::vector<std::vector<T>>& mat)
{
	static_assert(std::is_floating_point_v<T>, "T must be floating type");

	if (!strm.binary()) // ֻʵ��binaryģʽ�µĶ�ȡ
		return false;

	std::string sig;
	strm >> sig;
	if (sig != "DM" && sig != "FM")
		return false;

	std::int32_t rows, cols;
	if (!readBasicType(strm, rows) || !readBasicType(strm, cols))
		return false;

	mat.resize(rows);
	if (sig[0] == 'D' && sizeof(T) == sizeof(double) ||
		sig[0] == 'F' && sizeof(T) == sizeof(float)) { // ����һ�£�ֱ�Ӷ�ȡ
		for (std::int32_t i = 0; i < rows; i++) {
			mat[i].resize(cols);
			if (!strm.read(&mat[i][0], cols))
				return false;
		}
	}
	else if (sig[0] == 'D') { // doubleת��Ϊfloat��long double
		std::vector<double> tmp(cols);
		for (std::int32_t i = 0; i < rows; i++) {
			if (!strm.read(&tmp[0], cols))
				return false;
			stdx::copy(tmp, mat[i]);
		}
	}
	else { // floatת��Ϊdouble��long double
		std::vector<float> tmp(cols);
		for (std::int32_t i = 0; i < rows; i++) {
			if (!strm.read(&tmp[0], cols))
				return false;
			stdx::copy(tmp, mat[i]);
		}
	}

	return true;
}


template<typename T>
bool KuBasicIO::readTable(stdx::istreamx& strm, std::string& key, std::vector<std::vector<T>>& table)
{
	strm >> key;
	if (!strm) 
		return false;

	int c = strm->peek();
	if (c != ' ' && c != '\t' && c != '\n')
		return false;

	if (c != '\n') strm->get();  // Consume the space or tab.

	if (binaryTest(strm))
		strm.setBinary(true);

	return readFloatMatrix(strm, table);
}

