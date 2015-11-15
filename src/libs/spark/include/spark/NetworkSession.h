/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/MessageHandler.h>
#include <spark/SessionManager.h>
#include <spark/buffers/ChainedBuffer.h>
#include <logger/Logging.h>
#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>
#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace ember { namespace spark {

class NetworkSession : public std::enable_shared_from_this<NetworkSession> {
	const std::chrono::seconds SOCKET_ACTIVITY_TIMEOUT { 60 };
	const std::size_t HEADER_LENGTH = 1;
	const std::size_t MAX_MESSAGE_LENGTH = 1024 * 1024; // 1MB
	const std::size_t DEFAULT_BUFFER_LENGTH = 1024 * 16; // 16KB

	enum class ReadState { HEADER, BODY };

	boost::asio::ip::tcp::socket socket_;
	boost::asio::strand strand_;
	boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer_;

	ReadState state_;
	std::size_t body_read_size;
	std::vector<std::uint8_t> in_buff_;
	SessionManager& sessions_;
	MessageHandler handler_;
	log::Logger* logger_; 
	log::Filter filter_;

	bool process_header() {
		std::copy(in_buff_.begin(), in_buff_.begin() + HEADER_LENGTH, &body_read_size);
		boost::endian::little_to_native_inplace(body_read_size);

		if(body_read_size > MAX_MESSAGE_LENGTH) {
			LOG_WARN_FILTER(logger_, filter_)
				<< "[spark] Peer at " << remote_address() << ":"
				<< remote_port() << " attempted to send a message of "
				<< body_read_size << " bytes" << LOG_ASYNC;

			return false;
		}

		if(body_read_size > in_buff_.size()) {
			in_buff_.resize(body_read_size);
		}

		return true;
	}

	void read() {
		auto self(shared_from_this());
		
		std::size_t read_size = state_ == ReadState::HEADER? HEADER_LENGTH : body_read_size;

		boost::asio::async_read(socket_, boost::asio::buffer(in_buff_.data(), read_size),
			[this, self](boost::system::error_code ec, std::size_t size) {
				if(!ec) {
					set_timer();

					if(state_ == ReadState::HEADER) { // handle header
						if(process_header()) {
							state_ = ReadState::BODY;
							read();
						} else {
							close_session();
						}
					} else if(handler_.handle_message(in_buff_)) { // handle body
						state_ = ReadState::HEADER;
						read();
					} else {
						close_session();
					}
				} else if(ec != boost::asio::error::operation_aborted) {
					close_session();
				}
			}
		);
	}

	void set_timer() {
		timer_.expires_from_now(SOCKET_ACTIVITY_TIMEOUT);
		timer_.async_wait(strand_.wrap(std::bind(&NetworkSession::timeout, this,
		                                         std::placeholders::_1)));
	}

	void timeout(const boost::system::error_code& ec) {
		if(ec) { // if ec is set, the timer was aborted (session close / refreshed)
			return;
		}

		LOG_DEBUG_FILTER(logger_, filter_)
			<< "[spark] Lost connection to peer at " << remote_address() << ":"
			<< remote_port() << " (idle timeout) " << LOG_ASYNC;

		close_session();
	}

	void stop() {
		LOG_DEBUG_FILTER(logger_, filter_)
			<< "[spark] Closing connection to " << remote_address()
			<< ":" << remote_port() << LOG_ASYNC;

		boost::system::error_code ec; // we don't care about any errors
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		socket_.close(ec);
	}

public:
	NetworkSession(SessionManager& sessions, boost::asio::ip::tcp::socket socket, MessageHandler handler,
	               log::Logger* logger, log::Filter filter)
	               : sessions_(sessions), socket_(std::move(socket)), timer_(socket.get_io_service()),
	                 handler_(handler), strand_(socket.get_io_service()), logger_(logger), filter_(filter),
	                 state_(ReadState::HEADER), in_buff_(1000) { }

	void start() {
		set_timer();
		read();
	}

	void close_session() {
		sessions_.stop(shared_from_this());
	}

	std::string remote_address() {
		return socket_.remote_endpoint().address().to_string();
	}

	std::uint16_t remote_port() {
		return socket_.remote_endpoint().port();
	}

	template<std::size_t BlockSize>
	void write(std::shared_ptr<spark::ChainedBuffer<BlockSize>> chain) {
		auto self(shared_from_this());

		if(!socket_.is_open()) {
			return;
		}

		socket_.async_send(*chain,
			[this, self, chain](boost::system::error_code ec, std::size_t size) {
				chain->skip(size);

				if(ec && ec != boost::asio::error::operation_aborted) {
					close_session();
				}
			}
		);
	}

	virtual ~NetworkSession() = default;

	friend class SessionManager;
};

}} // spark, ember