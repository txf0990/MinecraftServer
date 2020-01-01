#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

using boost::asio::io_context;
using boost::asio::ip::tcp;
using std::cerr;
using std::cout;
using std::enable_shared_from_this;
using std::endl;
using std::list;
using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;

const int ROW_COUNT = 10;
const int COL_COUNT = 11;

typedef std::pair<int, int> Point;

// TODO: boost -> std
// TODO: bind -> &this->Function
// TODO: lock -> read-write-lock

// Thread safe
class Game {
public:
  // Contructor doesn't have to be thread safe.
  Game() : board_(ROW_COUNT, string(COL_COUNT, '.')) {
    for (int r = 0; r < ROW_COUNT; r++) {
      for (int c = 0; c < COL_COUNT; c++) {
        if (r == 0 || r == ROW_COUNT - 1 || c == 0 || c == COL_COUNT - 1) {
          board_[r][c] = '#';
        } else {
          available_positions_.push_back(std::make_pair(r, c));
        }
      }
    }
  }
  ~Game() {}

  shared_ptr<string> MakeSharedBoardMessage() {
    std::ostringstream board_str;
    Display(board_str);
    return make_shared<string>(board_str.str());
  }

  void Display(std::ostream& output) {
    lock_guard l(mutex_);
    output << "\033c";
    for (const auto& row : board_) {
      output << row << '\n';
    }
  }

  // Returns a random available position on board.
  Point AllocatePosition() {
    lock_guard l(mutex_);
    // TODO: find a random position
    size_t ind = std::rand() % available_positions_.size();
    Point p = available_positions_[ind];
    available_positions_[ind] = available_positions_.back();
    available_positions_.pop_back();
    board_[p.first][p.second] = '@';
    return p;
  }
  void PutBackPosition(Point p) {
    lock_guard l(mutex_);
    available_positions_.push_back(p);
    board_[p.first][p.second] = '.';
  }

private:
  vector<string> board_;  // GUARDED_BY(mutex_)
  vector<Point> available_positions_;  // GUARDED_BY(mutex_)
  std::mutex mutex_;
};

shared_ptr<string> MakeDaytimeString() {
  std::time_t now = std::time(0);
  return make_shared<string>(std::ctime(&now));
}

class Connection : public enable_shared_from_this<Connection> {
public:
  typedef std::function<void(shared_ptr<Connection>)> DisconnectHandler;

  ~Connection() {
    cerr << "Connection(" << point_.first << ", " << point_.second << ") distructed." << endl;
    game_.PutBackPosition(point_);
    on_board_change_();
  }

  template<typename... Args>
  static shared_ptr<Connection> Create(Args... args) {
    //return make_shared<Connection>(context);
    return shared_ptr<Connection>(new Connection(args...));
  }
  tcp::socket& Socket() {
    return socket_;
  }

  // Only called once when connection is accepted.
  void Start() {
    point_ = game_.AllocatePosition();
    cerr << "New connection started at (" << point_.first << ", " << point_.second << ")" << endl;
    //SendBoard();
    on_board_change_();
    Read();
  }

  Point Postion() {
    return point_;
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

private:
  // When connection is created, it's waiting for another connection with a vacant socket.
  // The connection is connected when Start() is called.
  Connection(boost::asio::io_context& context, Game& game, DisconnectHandler h,
             std::function<void()> board_change)
    : socket_(context), game_(game),
      on_disconnected_(h), on_board_change_(board_change){
    cerr << "Connection created." << endl;
  }
  
  void Disconnect() {
    if (connected_) {
      connected_ = false;
      on_disconnected_(shared_from_this());
    }
  }

  void SendBoard() {
    Write(game_.MakeSharedBoardMessage());
  }

  void Read() {
    if (!connected_) {
      cerr << "Connection(" << point_.first << ", " << point_.second
           << "): disconnect, do not continue reading." << endl;
      return;
    }
    cerr << "Waiting for data." << endl;
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
      cerr << "Error in Connection::HandleWrite" << error << endl;
      Disconnect();
      return;
    }
    cerr << "Message sent (" << bytes_transferred << "):" << *message << endl;
  }

  void HandleRead(
      const boost::system::error_code& error,
      size_t bytes_transferred) {
    if (error) {
      cerr << "Error in Connection::HandleRead" << error << endl;
      Disconnect();
      return;
    }
    std::string received(buf_.data(), bytes_transferred);

    cerr << "Message received : " << received << endl;

    auto message = MakeDaytimeString();
    message->pop_back();
    *message += "--->" + received + "\n";
    std::cerr << "Sending message: " << *message << std::endl;

    Write(message);
    Read();
  }

  bool connected_ = true;
  boost::array<char, 128> buf_;
  tcp::socket socket_;
  Game& game_;
  Point point_;
  DisconnectHandler on_disconnected_;
  std::function<void()> on_board_change_;  // broadcast board change to all clients.
};

class Server {
public:
  Server(io_context& context)
    : context_(context),
      acceptor_(context, tcp::endpoint(tcp::v4(), 10024)) {
    start_accept();
  }

  void start_accept() {
    auto connection = Connection::Create(std::ref(context_),
        std::ref(game_), 
        [this] (shared_ptr<Connection> c) { disconnect(c); },
        [this] () { Broadcast(); });
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
    cerr << "Connected." << endl;
    connections_.push_back(connection);
    if (!ec) {
      connection->Start();
    }
    start_accept();
  }

  void Broadcast() {
    auto message = game_.MakeSharedBoardMessage();

    for (const auto& p : connections_) {
      p->Write(message);
    }
  }

private:
  void disconnect(shared_ptr<Connection> connection) {
    cerr << "Remove connection from list." << endl;
    connections_.remove(connection);
  }
  io_context& context_;
  tcp::acceptor acceptor_;
  Game game_;
  list<shared_ptr<Connection>> connections_;
};


int main() {
  std::srand(std::time(nullptr));
  try {
    boost::asio::io_context context;
    Server server(context);
    context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  /*
  Game game;
  Point p = game.AllocatePosition();
  p = game.AllocatePosition();
  p = game.AllocatePosition();
  p = game.AllocatePosition();
  p = game.AllocatePosition();
  game.Display(cerr);
  cerr << p.first << ", " << p.second << endl;
  */

  return 0;
}
