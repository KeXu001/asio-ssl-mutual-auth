//
// client.cpp
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

enum {
    max_length = 1024
};

class client {
public:

    client(asio::io_service& io_service, asio::ssl::context& context,
            asio::ip::tcp::resolver::iterator endpoint_iterator)
    : socket_(io_service, context) {



        asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
        socket_.lowest_layer().async_connect(endpoint,
                bind(&client::handle_connect, this,
                std::placeholders::_1, ++endpoint_iterator));
    }

    std::string get_password() const {
        return "test";
    }

    void handle_connect(const std::error_code& error,
            asio::ip::tcp::resolver::iterator endpoint_iterator) {
        if (!error) {
            socket_.async_handshake(asio::ssl::stream_base::client,
                    bind(&client::handle_handshake, this,
                    std::placeholders::_1));
        } else if (endpoint_iterator != asio::ip::tcp::resolver::iterator()) {
            socket_.lowest_layer().close();
            asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
            socket_.lowest_layer().async_connect(endpoint,
                    bind(&client::handle_connect, this,
                    std::placeholders::_1, ++endpoint_iterator));
        } else {
            std::cout << "Connect failed: " << error << "\n";
        }
    }

    void handle_handshake(const std::error_code& error) {
        if (!error) {
            std::cout << "Enter message: ";
            std::cin.getline(request_, max_length);
            size_t request_length = strlen(request_);

            asio::async_write(socket_,
                    asio::buffer(request_, request_length),
                    bind(&client::handle_write, this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        } else {
            std::cout << "Handshake failed: " << error << "\n";
        }
    }

    void handle_write(const std::error_code& error,
            size_t bytes_transferred) {
        if (!error) {
            asio::async_read(socket_,
                    asio::buffer(reply_, bytes_transferred),
                    bind(&client::handle_read, this,
                    std::placeholders::_1,
                    std::placeholders::_2));
        } else {
            std::cout << "Write failed: " << error << "\n";
        }
    }

    void handle_read(const std::error_code& error,
            size_t bytes_transferred) {
        if (!error) {
            std::cout << "Reply: ";
            std::cout.write(reply_, bytes_transferred);
            std::cout << "\n";
        } else {
            std::cout << "Read failed: " << error << "\n";
        }
    }

private:
    asio::ssl::stream<asio::ip::tcp::socket> socket_;
    char request_[max_length];
    char reply_[max_length];
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: client <host> <port> <certfolder>\n";
            return 1;
        }
        
        const std::string cert_folder = argv[3];

        asio::io_service io_service;

        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query query(argv[1], argv[2]);
        asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);


        asio::ssl::context ctx(asio::ssl::context::sslv23);

        ctx.set_options(
                asio::ssl::context::default_workarounds
                | asio::ssl::context::no_sslv2
                | asio::ssl::context::single_dh_use);


        ctx.set_verify_mode(asio::ssl::context::verify_peer | asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.load_verify_file("certs/server.crt");

        ctx.use_certificate_chain_file(cert_folder+"/server.crt");
        ctx.use_private_key_file(cert_folder+"/server.key", asio::ssl::context::pem);
        ctx.use_tmp_dh_file(cert_folder+"/dh512.pem");

        client c(io_service, ctx, iterator);
        io_service.run();
    } catch (std::error_code& e) {
        std::cerr << "Exception: " << e.message() << "\n";
    }

    return 0;
}
