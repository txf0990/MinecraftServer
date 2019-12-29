#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return std::ctime(&now);
}

int main() {
  try {
    boost::asio::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 10024));

    boost::system::error_code error;
    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);


      std::string greeting = "Hello!\n";
      boost::asio::write(socket, boost::asio::buffer(greeting), error);

      boost::array<char, 128> buf;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error)
        throw boost::system::system_error(error); // Some other error.

      std::string receivedMessage(buf.data(), len);
      std::string message = make_daytime_string() + "--->" + receivedMessage;
      std::cout << "Send message: " << message << std::endl;

      boost::asio::write(socket, boost::asio::buffer(message), error);
      if (error)
        throw boost::system::system_error(error); // Some other error.
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
