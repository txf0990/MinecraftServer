#include <iostream>
#include <thread>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
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
          connect(host, port);
        }

    void start(shared_ptr<string> message) {
      send(message);
      receive();
    }

  protected:
    void connect(const char* host, const char* port) {
      try { 
        tcp::resolver resolver(context_);
        tcp::resolver::results_type endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket_, endpoints);
        cout << "Connection established!" << endl;
      }
      catch (...) {
        cout << "Cannot establish connection!" << endl;
        throw;
      }
    }

    void receive() {
      cout << "receive(): Waiting for data." << endl;
      socket_.async_receive(
          boost::asio::buffer(buf_),
          boost::bind(
            &Client::handle_receive,
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    void handle_receive(
        const boost::system::error_code& error,
        size_t bytes_transferred) {
      cout << "handle_receive()." << endl; 
      if (error) {
        cout << error << endl;
        return;
      }
      std::string received(buf_.data(), bytes_transferred);
      cout << "Message received : " << received << endl;
      receive();
    }
    
    void send(shared_ptr<string> message) {
      cout << "send(): Sending data." << endl;
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
    void handle_send(
        shared_ptr<string> message,
        const boost::system::error_code& error,
        size_t bytes_transferred) {
      cout << "handle_send()." << endl; 
      if (error) {
        cout << error <<endl;
      }
      cout << "Message sent: (" << bytes_transferred << "): " << *message << endl;
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
    
    string input;

    int loop_cnt = 0;
    while(true)
    {
      loop_cnt++;
      cin >> input;
      client.start(make_shared<string>(input));
      if (loop_cnt == 1) {
        std::thread t(boost::bind(&boost::asio::io_context::run, &context));
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

/*



int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      cerr << "Usage: client <host> <port>" << endl;
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve(argv[1], argv[2]);

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    for (;;)
    {
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if (error)
        throw boost::system::system_error(error); // Some other error.

      std::cout.write(buf.data(), len);

      std::string message;
      std::cin >> message;

      std::cout << "Writing message: " << message << std::endl;
      boost::asio::write(socket, boost::asio::buffer(message), error);

      if (error) {
        throw boost::system::system_error(error); // Some other error.
      } else {
        std::cout << "Message written." << std::endl;
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

*/
