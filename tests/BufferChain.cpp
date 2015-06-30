/*
 * Copyright (c) 2014 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spark/BufferChain.h>
#include <gtest/gtest.h>
#include <memory>

namespace spark = ember::spark;

TEST(BufferChainTest, Size) {
	spark::BufferChain<32> chain;
	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";

	chain.reserve(50);
	ASSERT_EQ(50, chain.size()) << "Chain size is incorrect";

	char buffer[20];
	chain.read(buffer, sizeof(buffer));
	ASSERT_EQ(30, chain.size()) << "Chain size is incorrect";

	chain.skip(10);
	ASSERT_EQ(20, chain.size()) << "Chain size is incorrect";

	chain.write(buffer, 20);
	ASSERT_EQ(40, chain.size()) << "Chain size is incorrect";

	chain.clear();
	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
}

TEST(BufferChainTest, ReadWriteConsistency) {
	spark::BufferChain<32> chain;
	char text[] = "The quick brown fox jumps over the lazy dog";
	int num = 41521;

	chain.write(text, sizeof(text));
	chain.write(&num, sizeof(int));

	auto text_out = std::make_unique<char[]>(sizeof(text));
	int num_out;

	chain.read(text_out.get(), sizeof(text));
	chain.read(&num_out, sizeof(int));

	std::memcmp(text, text_out.get(), sizeof(text));
	ASSERT_EQ(num, num_out) << "Read produced incorrect result";
	ASSERT_STREQ(text, text_out.get()) << "Read produced incorrect result";
	ASSERT_EQ(0, chain.size()) << "Chain should be empty";
}

TEST(BufferChainTest, ReserveFetchConsistency) {
	spark::BufferChain<32> chain;
	char text[] = "The quick brown fox jumps over the lazy dog";
	const std::size_t text_len = sizeof(text);

	chain.reserve(text_len);
	ASSERT_EQ(text_len, chain.size()) << "Chain size is incorrect";

	auto buffers = chain.fetch_buffers(text_len);
	std::string output; output.resize(text_len);

	// store text in the retrieved buffers
	std::size_t offset = 0;

	for(auto& buffer : buffers) {
		std::memcpy(buffer->buff.data(), text + offset, buffer->size());
		offset += buffer->size();

		if(offset > text_len || !offset) {
			break;
		}
	}

	// read text back out
	chain.read(&output[0], text_len);

	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
	ASSERT_STREQ(text, output.c_str()) << "Read produced incorrect result";
}

TEST(BufferChainTest, Skip) {
	spark::BufferChain<32> chain;
	int foo = 960;
	int bar = 296;

	chain.write(&foo, sizeof(int));
	chain.write(&bar, sizeof(int));
	chain.skip(sizeof(int));
	chain.read(&foo, sizeof(int));

	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
	ASSERT_EQ(bar, foo) << "Skip produced incorrect result";
}

TEST(BufferChainTest, Clear) {
	spark::BufferChain<32> chain;
	const int iterations = 100;

	for(int i = 0; i < iterations; ++i) {
		chain.write(&i, sizeof(int));
	}

	ASSERT_EQ(sizeof(int) * iterations, chain.size()) << "Chain size is incorrect";
	chain.clear();
	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
}

TEST(BufferChainTest, AttachTail) {
	spark::BufferChain<32> chain;
	spark::Buffer<32>* buffer = chain.allocate();

	std::string text("This is a string that is almost certainly longer than 32 bytes");
	std::size_t written = buffer->write(text.c_str(), text.length());

	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
	chain.attach(buffer);
	ASSERT_EQ(written, chain.size()) << "Chain size is incorrect";

	std::string output; output.resize(written);

	chain.read(&output[0], written);
	ASSERT_EQ(0, chain.size()) << "Chain size is incorrect";
	ASSERT_EQ(0, std::memcmp(text.data(), output.data(), written)) << "Output is incorrect";
}

TEST(BufferChainTest, RetrieveTail) {
	spark::BufferChain<32> chain;
	std::string text("This string is < 32 bytes"); // could this fail on exotic platforms?
	chain.write(text.data(), text.length());

	auto tail = chain.tail();
	ASSERT_EQ(0, std::memcmp(text.data(), tail->buff.data(), text.length())) << "Tail data is incorrect";
}

TEST(BufferChainTest, Copy) {
	spark::BufferChain<32> chain;
	int output, foo = 54543;
	chain.write(&foo, sizeof(int));
	ASSERT_EQ(sizeof(int), chain.size());
	chain.copy(&output, sizeof(int));
	ASSERT_EQ(sizeof(int), chain.size()) << "Chain size is incorrect";
	ASSERT_EQ(foo, output) << "Copy output is incorrect";
}

TEST(BufferChainTest, CopyChain) {
	spark::BufferChain<32> chain, chain2;

}

TEST(BufferChainTest, MoveChain) {
	spark::BufferChain<32> chain, chain2;

}
