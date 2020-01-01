#include <iostream>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::make_shared;
using std::shared_ptr;
using boost::asio::io_context;

class Client {
public:
  Client(io_context& context, const char* host, const char* port)
    : context_(context),
      socket_(context) {
    // Connect server
    try {
      tcp::resolver resolver(context_);
      tcp::resolver::results_type endpoints = resolver.resolve(host, port);
      boost::asio::connect(socket_, endpoints);
      cerr << "Connection established!" << endl;
    }
    catch (...) {
      cerr << "Cannot establish connection!" << endl;
      throw;
    }
  }

  void receive() {
    cerr << "receive(): Waiting for data." << endl;
    socket_.async_receive(
        boost::asio::buffer(buf_),
        boost::bind(
          &Client::handle_receive,
          this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void send(shared_ptr<string> message) {
    cerr << "send(): Sending data." << endl;
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*message),
        boost::bind(
            &Client::handle_send,
            this,
            message,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
  }

private:
  void handle_receive(
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    cerr << "handle_receive()." << endl; 
    if (error == boost::asio::error::eof) {
      throw boost::system::system_error(error);
    }
    if (error) {
      throw boost::system::system_error(error);
    }
    cerr << "Message received (" << bytes_transferred << ")." << endl;
    cout.write(buf_.data(), bytes_transferred);
    receive();
  }
  
  void handle_send(
      shared_ptr<string> message,
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    cerr << "handle_send()." << endl; 
    if (error) {
      throw boost::system::system_error(error);
    }
    cerr << "Message sent: (" << bytes_transferred << "): " << *message << endl;
  }

  
  boost::asio::io_context& context_;
  tcp::socket socket_;
  boost::array<char, 128> buf_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: client <host> <port>" << endl;
      return 1;
    }
    boost::asio::io_context context;
    Client client(context, argv[1], argv[2]);
    client.receive();
    
    std::thread t(boost::bind(&boost::asio::io_context::run, &context));

    while(true)
    {
      auto input = make_shared<string>();
      cin >> *input;
      client.send(input);
    }
  }
  catch (std::exception& e)
  {
    cerr << "Exception caught in main(): " << e.what() << std::endl;
  }
}

