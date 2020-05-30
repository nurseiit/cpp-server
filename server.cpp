#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <string>

#define PORT 80

using asio::ip::tcp;

std::string build_message() { return "Hello from Server!"; }

int main() {
  try {
    asio::io_context io_context;

    // Listen on 0.0.0.0:PORT
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));

    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::string message = build_message();

      asio::error_code error;
      asio::write(socket, asio::buffer(message), error);
      std::cerr << error.value() << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}