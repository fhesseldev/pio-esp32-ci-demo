#ifndef cidemolib_hpp
#define cidemolib_hpp

#include <Arduino.h>

#include "lwip/sockets.h"

#include "stdint.h"
#include "stdbool.h"

#define CHAT_SERVER_MAXCON 10

class ChatServer {
public:
  /** Creates an instance of the echo server */
  ChatServer(uint16_t port);
  /** Destroys the echo server */
  virtual ~ChatServer();
  /** Starts the server */
  void start();
  /** Processes client. Call it in the main programs loop */
  void loop();
  /** Stops the servers */
  void stop();

private:
  /** Function to handle incoming connections */
  void handleIncoming(size_t freeConID);
  /** Function to handle existing connection */
  void handleConnection(size_t conID);
  /** Port to bind to */
  uint16_t _port;
  /** Server socket */
  int _socket;
  /** Server socket address */
  sockaddr_in _sock_addr;
  /** Client sockets */
  int _client[CHAT_SERVER_MAXCON];
};

#endif // cidemolib_hpp