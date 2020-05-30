#include <asio.hpp>
#include <iostream>
#include <istream>
#include <map>
#include <ostream>
#include <string>

using asio::ip::tcp;

class Session;

class Headers {
  std::string method;
  std::string url;
  std::string version;

  std::map<std::string, std::string> headers_map;

public:
  std::string get_response() {
    std::stringstream response_stream;
    // TODO read from files
    if (url == "/") {
      std::string content = "<html><body><h1>Hello World</h1><p>This is a web "
                            "server in using asio</p></body></html>";
      response_stream << "HTTP/1.1 200 OK" << std::endl;
      response_stream << "content-type: text/html" << std::endl;
      response_stream << "content-length: " << content.length() << std::endl;
      response_stream << std::endl;
      response_stream << content;
    } else {
      std::string content = "<html><body><h1>404 Not Found</h1></body></html>";
      response_stream << "HTTP/1.1 404 Not Found" << std::endl;
      response_stream << "content-type: text/html" << std::endl;
      response_stream << "content-length: " << content.length() << std::endl;
      response_stream << std::endl;
      response_stream << content;
    }
    return response_stream.str();
  }

  int content_length() {
    if (headers_map.count("content-length")) {
      std::stringstream content_length_stream(headers_map["content-length"]);
      int content_length;
      content_length_stream >> content_length;
      return content_length;
    }
    return 0;
  }

  void read_header(std::string line) {
    std::stringstream header_stream(line);
    std::string header_name;
    std::getline(header_stream, header_name, ':');

    std::string header_value;
    std::getline(header_stream, header_value);
    headers_map[header_name] = header_value;
  }

  void read_request_line(std::string line) {
    std::stringstream request_stream(line);
    request_stream >> method;
    request_stream >> url;
    request_stream >> version;

    std::cout << "🙈 Requested: " << url << std::endl;
  }
};

class Session {
  asio::streambuf buf;
  Headers headers;

  static void read_body(std::shared_ptr<Session> ptr) {
    int nbuffer = 6969;
    std::shared_ptr<std::vector<char>> bufptr =
        std::make_shared<std::vector<char>>(nbuffer);
    asio::async_read(ptr->socket, asio::buffer(*bufptr, nbuffer),
                     [ptr](const asio::error_code &e, std::size_t s) {});
  }

  static void read_next_line(std::shared_ptr<Session> ptr) {
    asio::async_read_until(
        ptr->socket, ptr->buf, '\r',
        [ptr](const asio::error_code &e, std::size_t s) {
          std::string line, _;
          std::istream stream{&ptr->buf};
          std::getline(stream, line, '\r');
          std::getline(stream, _, '\n');
          ptr->headers.read_header(line);

          if (line.length() == 0) {
            if (ptr->headers.content_length() == 0) {
              std::shared_ptr<std::string> str =
                  std::make_shared<std::string>(ptr->headers.get_response());
              asio::async_write(
                  ptr->socket, asio::buffer(str->c_str(), str->length()),
                  [ptr, str](const asio::error_code &e, std::size_t s) {
                    std::cout << "👌 done" << std::endl;
                  });
            } else {
              ptr->read_body(ptr);
            }
          } else {
            ptr->read_next_line(ptr);
          }
        });
  }

  static void read_first_line(std::shared_ptr<Session> ptr) {
    asio::async_read_until(ptr->socket, ptr->buf, '\r',
                           [ptr](const asio::error_code &e, std::size_t s) {
                             std::string line, _;
                             std::istream stream{&ptr->buf};
                             std::getline(stream, line, '\r');
                             std::getline(stream, _, '\n');
                             ptr->headers.read_request_line(line);
                             ptr->read_next_line(ptr);
                           });
  }

public:
  tcp::socket socket;

  Session(asio::io_service &io_service) : socket(io_service) {}

  static void start(std::shared_ptr<Session> ptr) { read_first_line(ptr); }
};

void accept_and_run(tcp::acceptor &acceptor, asio::io_service &io_service) {
  std::shared_ptr<Session> sesh = std::make_shared<Session>(io_service);
  acceptor.async_accept(
      sesh->socket,
      [sesh, &acceptor, &io_service](const asio::error_code &accept_error) {
        accept_and_run(acceptor, io_service);
        if (!accept_error) {
          Session::start(sesh);
        }
      });
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <port>\n";
    std::cout << "Examples:\n";
    std::cout << "  " << argv[0] << " 8080\n";
    return EXIT_FAILURE;
  }

  int PORT = std::atoi(argv[1]);

  asio::io_service io_service;
  tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), PORT));

  acceptor.listen();
  accept_and_run(acceptor, io_service);

  io_service.run();
  return EXIT_SUCCESS;
}