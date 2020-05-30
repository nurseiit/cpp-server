#include <asio.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

using asio::ip::tcp;

// Parse path from given url
std::string path_from_url(const std::string &str) {
  int pos = str.find_first_of("/");
  if (pos == -1)
    return "/";
  return str.substr(pos, str.length() - pos);
}

// Parse hostname from given url
std::string hostname_from_url(const std::string &str) {
  int pos = str.find_first_of("/");
  if (pos == -1)
    return str;
  return str.substr(0, pos);
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cout << "Usage: " << argv[0] << " <url> <port>\n";
      std::cout << "Examples:\n";
      std::cout << "  " << argv[0] << " www.example.com 80\n";
      std::cout << "  " << argv[0] << " www.google.com/test.html 80\n";
      return EXIT_FAILURE;
    }

    std::string hostname = hostname_from_url(argv[1]);
    std::string path = path_from_url(argv[1]);
    std::string port = argv[2];

    asio::io_context io_context;

    // Get endpoints for server name
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(hostname, port);

    // Find first successful endpoint
    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);

    // Set bare minimum request headers for GET to work
    // https://stackoverflow.com/questions/6686261/what-at-the-bare-minimum-is-required-for-an-http-request
    asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << hostname << "\r\n";
    // Set "Connection: close" so server ends response with EOF
    request_stream << "Connection: close\r\n\r\n";

    // Send request
    asio::write(socket, request);

    // Read response status line
    asio::streambuf response;
    asio::read_until(socket, response, "\r\n");

    // Check response
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      std::cout << "Invalid response\n";
      return EXIT_FAILURE;
    }

    std::cout << "STATUS CODE: " << status_code << std::endl;

    // Read & Print to console response headers
    asio::read_until(socket, response, "\r\n\r\n");

    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

    // Read and print response till EOF
    if (response.size() > 0)
      std::cout << &response;

    asio::error_code error;
    while (asio::read(socket, response, asio::transfer_at_least(1), error))
      std::cout << &response;

    // If read throws EOF, it's expected
    if (error != asio::error::eof)
      throw asio::system_error(error);
  } catch (std::exception &e) {
    std::cout << "ERROR: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}