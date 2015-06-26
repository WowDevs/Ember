/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/BufferChainNode.h>
#include <boost/assert.hpp>
#include <cstddef>
#include <iostream>
#include <vector>
#include <utility>

namespace ember { namespace spark {

typedef std::pair<const char*, std::size_t> ConstRawBuffer;
typedef std::pair<char*, std::size_t> RawBuffer;

template<typename std::size_t BlockSize>
class BufferChain {
	BufferChainNode root_;
	std::size_t size_;

	void link_tail_node(BufferChainNode* node) {
		node->next = &root_;
		node->prev = root_.prev;
		root_.prev = root_.prev->next = node;
	}

	void allocate_tail_node() {

	}

	void unlink_node(BufferChainNode* node) {
		auto* next = node->next;
		auto* prev = node->prev;
		next->prev = prev;
		prev->next = next;
	}

	Buffer<BlockSize>* buffer_from_node(BufferChainNode* node) {
		return reinterpret_cast<Buffer<BlockSize>*>(std::size_t(node) - offsetof(Buffer<BlockSize>, node));
	}
	
public:
	BufferChain() {
		root_.next = &root_;
		root_.prev = &root_;
		size_ = 0;
	}

	~BufferChain() {
		BufferChainNode* head;

		while((head = root_.next) != &root_) {

		}
	}

	void read(char* destination, std::size_t length) {
		BOOST_ASSERT_MSG(length <= size_, "Chained buffer read too large!");
		std::size_t remaining = length;

		while(remaining) {
			auto head = buffer_from_node(root_.next);
			remaining -= head->read(destination + length - remaining, remaining);

			if(remaining) {
				unlink_node(root_.next);
			}
		}

		size_ -= length;
	}

	void copy(const char* source, std::size_t length) {
		BOOST_ASSERT_MSG(length <= size_, "Chained buffer copy too large!");
		std::size_t remaining = length;
		auto head = root_.next;

		while(remaining) {
			auto buffer = buffer_from_node(head);
			remaining -= buffer->copy(destination + length - remaining, remaining);

			if(remaining) {
				head = head->next;
			}
		}
	}

	std::vector<ConstRawBuffer> fetch_buffers(std::size_t length) {
		BOOST_ASSERT_MSG(length <= size_, "Chained buffer fetch too large!");
		std::vector<RawBuffer> buffers;
		auto head = root_.next;

		while(length) {
			auto buffer = buffer_from_node(head);
			auto size = buffer->size();
			buffers.emplace_back(buffer->buff.data(), size);
			length -= size;
			head = head->next;
		}

		return buffers;
	}

	void skip(std::size_t length) {
		BOOST_ASSERT_MSG(length <= size_, "Chained buffer skip too large!");
		auto head = root_.next;

		while(length) {
			auto buffer = buffer_from_node(head);
			length -= buffer->size();

			if(length) {
				unlink_node(head);
			}
		}
	}

	void write(const char* source, std::size_t length) {
		std::size_t remaining = length;
		BufferChainNode* tail = root_.prev;

		while(remaining) {
			if(tail == &root_) {
				Buffer<BlockSize>* buffer = new Buffer<BlockSize>();
				link_tail_node(&buffer->node);
				tail = root_.prev;
			}

			auto buffer = buffer_from_node(tail);
			remaining -= buffer->write(source + length - remaining, remaining);
			tail = tail->next;
		}

		size_ += length;
	}

	std::size_t size() {
		return size_;
	}

	RawBuffer tail() {
		return { root_.next, buffer_from_node(root_.next)->size() };
	}

	void attach(Buffer<BlockSize>* buffer) {
		link_tail_node(buffer->node);
	}
};

}} // spark, ember