#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::io_context;
using boost::asio::ip::tcp;
using std::cout;
using std::enable_shared_from_this;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::string;

//const string GREETING("Hello\n");

shared_ptr<string> MakeDaytimeString() {
  std::time_t now = std::time(0);
  return make_shared<string>(std::ctime(&now));
}

class Connection : public enable_shared_from_this<Connection> {
public:
  ~Connection() {
    cout << "Connection distructed." << endl;
  }

  static shared_ptr<Connection> Create(io_context& context) {
    //return make_shared<Connection>(context);
    return shared_ptr<Connection>(new Connection(context));
  }
  tcp::socket& Socket() {
    return socket_;
  }

  void Start() {
    Write(make_shared<string>("Hello!\n"));
    Read();
  }
private:
  Connection(boost::asio::io_context& context)
    : socket_(context) {
    cout << "Connection created." << endl;
  }

  void Write(const shared_ptr<string>& message) {
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*message),
        boost::bind(
          &Connection::HandleWrite,
          shared_from_this(),
          message,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  void Read() {
    cout << "Waiting for data." << endl;
    socket_.async_receive(
        boost::asio::buffer(buf_),
        boost::bind(
          &Connection::HandleRead,
          shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  void HandleWrite(
      shared_ptr<string> message,
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    if (error) {
      // TODO: try cout << error << endl;
      //throw boost::system::system_error(error);
      cout << error << endl;
    }
    cout << "Message sent (" << bytes_transferred << "):" << *message << endl;
  }

  void HandleRead(
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    if (error) {
      // TODO: try cout << error << endl;
      cout << error << endl;
      return;
    }
    std::string received(buf_.data(), bytes_transferred);

    cout << "Message received : " << received << endl;

    auto message = MakeDaytimeString();
    message->pop_back();
    *message += "--->" + received + "\n";
    std::cout << "Sending message: " << *message << std::endl;

    Write(message);
    Read();
  }

  boost::array<char, 128> buf_;
  tcp::socket socket_;
};

class Server {
public:
  Server(io_context& context)
    : context_(context),
      acceptor_(context, tcp::endpoint(tcp::v4(), 10024)) {
    start_accept();
  }

  void start_accept() {
    auto connection = Connection::Create(context_);
    acceptor_.async_accept(
        connection->Socket(),
        boost::bind(&Server::handle_accept,
                    this,
                    connection,
                    boost::asio::placeholders::error));
  }

  void handle_accept(
    shared_ptr<Connection> connection,
    const boost::system::error_code& ec) {
    cout << "Connected." << endl;
    if (!ec) {
      connection->Start();
    }
    start_accept();
  }
private:
  io_context& context_;
  tcp::acceptor acceptor_;
};


int main() {
  try {
    boost::asio::io_context context;
    Server server(context);
    context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
