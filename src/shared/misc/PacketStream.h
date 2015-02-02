#pragma once

#include <string>
#include <vector>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>

namespace ember {

typedef std::vector<char> Packet;

template<typename T>
class PacketStream {
	T& buffer_;

public:
	PacketStream(T& buffer) : buffer_(buffer) {}

	PacketStream& operator <<(const std::string& data) {
		std::copy(data.begin(), data.end(), std::back_inserter(buffer_));
		return *this;
	}

	template<typename U>
	PacketStream& operator <<(const U& data) {
		const char* copy = static_cast<const char*>(static_cast<const void*>(&data));
		std::copy(copy, copy + sizeof(data), std::back_inserter(buffer_));
		return *this;
	}
};

} //ember