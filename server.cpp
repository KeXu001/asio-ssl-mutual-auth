//
// server.cpp
// ~~~~~~~~~~
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <functional>
#include <system_error>
#include <asio.hpp>
#include <asio/ssl.hpp>

typedef asio::ssl::stream<asio::ip::tcp::socket> ssl_socket;

class session {
public:

    session(asio::io_service& io_service, asio::ssl::context& context)
    : socket_(io_service, context) {
    }

    ssl_socket::lowest_layer_type& socket() {
        return socket_.lowest_layer();
    }

    void start() {
        socket_.async_handshake(asio::ssl::stream_base::server,
                bind(&session::handle_handshake, this,
                std::placeholders::_1));
    }

    void handle_handshake(const std::error_code& error) {
        if (!error) {
            socket_.async_read_some(asio::buffer(data_, max_length),
                    bind(&session::handle_read, this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        } else {
            delete this;
        }
    }

    void handle_read(const std::error_code& error,
            size_t bytes_transferred) {
        if (!error) {
            asio::async_write(socket_,
                    asio::buffer(data_, bytes_transferred),
                    bind(&session::handle_write, this,
                    std::placeholders::_1));
        } else {
            delete this;
        }
    }

    void handle_write(const std::error_code& error) {
        if (!error) {
            socket_.async_read_some(asio::buffer(data_, max_length),
                    bind(&session::handle_read, this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        } else {
            delete this;
        }
    }

private:
    ssl_socket socket_;

    enum {
        max_length = 1024
    };
    char data_[max_length];
};

class server {
public:

    server(asio::io_service& io_service, unsigned short port)
    : io_service_(io_service),
    acceptor_(io_service,
    asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
    context_(asio::ssl::context::sslv23) {
        
        
        
        context_.set_options(
                asio::ssl::context::default_workarounds
                | asio::ssl::context::no_sslv2
                | asio::ssl::context::single_dh_use);
        
        //context_.set_password_callback(bind(&server::get_password, this));

        context_.use_certificate_chain_file("server_certs/server.crt");
        context_.use_private_key_file("server_certs/server.key", asio::ssl::context::pem);
        context_.use_tmp_dh_file("dh/dhparam.pem");
        
        /**
         * verify client auth
         */
        context_.set_verify_mode(asio::ssl::context::verify_fail_if_no_peer_cert | asio::ssl::context::verify_peer);
        context_.load_verify_file("ca_certs/ca.crt");
        
        session* new_session = new session(io_service_, context_);
        acceptor_.async_accept(new_session->socket(),
                bind(&server::handle_accept, this, new_session,
                std::placeholders::_1));
    }

    // std::string get_password() const {
    //     return "test";
    // }

    void handle_accept(session* new_session,
            const std::error_code& error) {
        if (!error) {
            new_session->start();
            new_session = new session(io_service_, context_);
            acceptor_.async_accept(new_session->socket(),
                    bind(&server::handle_accept, this, new_session,
                    std::placeholders::_1));
        } else {
            delete new_session;
        }
    }

private:
    asio::io_service& io_service_;
    asio::ip::tcp::acceptor acceptor_;
    asio::ssl::context context_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }

        asio::io_service io_service;

        using namespace std; // For atoi.
        server s(io_service, atoi(argv[1]));

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
