#pragma once
#include <istream>
#include <vector>
#include <assert.h>
#include "KmEndianed.h"
#include "../graph/base/KtHolder.h"


namespace stdx 
{

	/// std::istream的扩展

	class istreamx : public KmEndianed
	{
	public:

		istreamx(std::istream& strm, bool bin) 
			: strm_(strm), bin_(bin) {}

		istreamx(istreamx& strmx)
			: strm_(strmx.strm_), bin_(strmx.bin_) {}


		// 读取标量数据
		template<class T>
		typename std::enable_if_t<std::is_scalar<T>::value, istreamx&>
		read(T& val) {
			if (!bin_)
				strm_ >> val;
			else if (strm_.read(reinterpret_cast<char*>(&val), sizeof(T))) 
			    matchEndian(val);
				
			return *this;
		}

		template<typename T>
		istreamx& read(KtHolder<T>& val) {
			return read(val.inside());
		}

		template<typename T, typename U>
		istreamx& read(std::pair<T, U>& val) {
			return read(val.first), read(val.second);
		}

		template<typename... T>
		istreamx& read(std::tuple<T...>& val) {
			return readTuple_(val);
		}

		template<typename T, typename SIZE_TYPE = std::uint32_t>
		typename std::enable_if_t<std::is_scalar<T>::value, istreamx&>
		read(std::vector<T>& val) {
			if (bin_) {
				SIZE_TYPE c;
				if (read(c)) {
					val.resize(c);
					if (c > 0) read(&val[0], c);
				}
			}
			else {
				val.clear();
				auto pos = strm_.tellg();
				while (true) {
					T x;
					if (!read(x)) break;

					val.push_back(x);
					pos = strm_.tellg();
				}
				
				strm_.seekg(pos);
				strm_.unsetf(std::istream::failbit);
			}

			return *this;
		}


		// 读取数组
		template<typename T>
		typename std::enable_if_t<std::is_scalar<T>::value, istreamx&>
		read(T* val, unsigned count) {
			if (bin_) {
				if(strm_.read(reinterpret_cast<char*>(val), sizeof(T) * count))
				    matchEndian(val, count);
			}
			else {
				while (strm_ && count-- != 0) 
					strm_ >> std::ws >> *val++;
			}

			return *this;
		}

		istreamx& read(std::string& val) {
			if (!bin_) strm_ >> std::ws;
			strm_ >> val;

			if (!isspace(strm_.peek()))
				strm_.setf(std::istream::failbit);
			else if (bin_)
				strm_.get(); // consume the space

			return *this;
		}

		template<typename T>
		typename std::enable_if_t<std::is_scalar<T>::value, T>
		peek() {
			// assert(strm_.openmode & std::ios_base::binary);
			T val{};
			auto pos = strm_.tellg();
			read(val);
			strm_.seekg(pos);
			assert(strm_.tellg() == pos);
			return val;
		}


		// 读取以长度开头的字符串，无'\0'结尾
		template<typename CHAR, typename SIZE_TYPE = std::uint32_t>
		istreamx& readBinString(std::basic_string<CHAR>& retString) {
			SIZE_TYPE len = 0;
			if (read(len)) {
				retString.resize(len);
				return read(&retString[0], len);
			}

			return *this;
		}


		bool operator !() { return !strm_; }
		operator bool() { return bool(strm_); }

		std::istream* operator->() { return &strm_; }
		operator std::istream& () { return strm_; }

		bool binary() const { return bin_; }
		void setBinary(bool b) { bin_ = b; }

	private:

		template<typename TUPLE_TYPE, int INDEX = std::tuple_size_v<TUPLE_TYPE>>
		istreamx& readTuple_(TUPLE_TYPE& val) {
			if constexpr (INDEX == 1) {
				return read(std::get<0>(val));
			}
			else {
				readTuple_<TUPLE_TYPE, INDEX - 1>(val);
				return read(std::get<INDEX - 1>(val));
			}
		}

	private:
		std::istream& strm_;
		bool bin_; // 是否二进制读写模式
	};


	template<typename T>
	istreamx& operator >>(istreamx& strm, T& val) { 
		return strm.read(val);
	}
}