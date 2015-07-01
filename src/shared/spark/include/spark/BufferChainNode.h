/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#pragma warning(disable : 4996)

#include <array>
#include <cstddef>

namespace ember { namespace spark {

struct BufferChainNode {
	BufferChainNode* next;
	BufferChainNode* prev;
};

template<typename std::size_t BlockSize>
struct BufferBlock {
	std::array<char, BlockSize> buff;
	std::size_t read_offset = 0;
	std::size_t write_offset = 0;
	BufferChainNode node;

	void reset() {
		read_offset = 0;
		write_offset = 0;
	}

	std::size_t write(const char* source, std::size_t length) {
		std::size_t write_len = BlockSize - write_offset;

		if(write_len > length) {
			write_len = length;
		}

		std::copy(source, source + write_len, buff.data() + write_offset);
		write_offset += write_len;
		return write_len;
	}

	std::size_t copy(char* destination, std::size_t length) {
		std::size_t read_len = BlockSize - read_offset;

		if(read_len > length) {
			read_len = length;
		}

		std::copy(buff.data() + read_offset, buff.data() + read_offset + read_len, destination);
		return read_len;
	}

	std::size_t read(char* destination, std::size_t length) {
		std::size_t read_len = copy(destination, length);
		read_offset += read_len;

		if(read_offset == write_offset) {
			reset();
		}

		return read_len;
	}

	std::size_t skip(std::size_t length) {
		std::size_t skip_len = BlockSize - read_offset;

		if(skip_len > length) {
			skip_len = length;
		}

		read_offset += skip_len;

		if(read_offset == write_offset) {
			reset();
		}

		return skip_len;
	}

	std::size_t reserve(std::size_t length) {
		std::size_t reserve_len = BlockSize - write_offset;

		if(reserve_len > length) {
			reserve_len = length;
		}

		write_offset += reserve_len;
		return reserve_len;
	}

	std::size_t size() {
		return write_offset;
	}
};

}} // spark, ember