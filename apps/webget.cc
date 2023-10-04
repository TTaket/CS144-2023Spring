#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <random>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  // cerr << "Warning: get_URL() has not been implemented yet.\n";

  // 1. 建立套接字
  TCPSocket sock;
  // 2. 确定链接地址
  // Address的三种构造方法
  // Address( const std::string& hostname, const std::string& service );
  // explicit Address( const std::string& ip, std::uint16_t port = 0 );
  // Address( const sockaddr* addr, std::size_t size );
  Address ad( host, "http" );

  // 3. 建立连接  Socket类中void connect( const Address& address );
  sock.connect( ad );

  // 4. 输入信息
  // 输入GET /hello HTTP/1.1。这将告诉服务器URL的路径部分。
  sock.write( "GET " + path + " HTTP/1.1" + "\r\n" );
  // 输入Host: cs144.keithw.org。这将告诉服务器URL的主机部分
  sock.write( "Host: " + host + "\r\n" );
  // 设置短链接
  sock.write( "Connection: close\r\n\r\n" );
  string outinfo;
  sock.read( outinfo );
  while ( !outinfo.empty() ) {
    cout << outinfo;
    sock.read( outinfo );
  }
  sock.close();
  // cout << "完成任务" <<endl;
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    // auto args = std::span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << argv[0] << " HOST PATH\n";
      cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { argv[1] };
    const string path { argv[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
