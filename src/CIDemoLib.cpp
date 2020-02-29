#include "CIDemoLib.hpp"

#include "lwip/inet.h"

ChatServer::ChatServer(uint16_t port) {
  _socket = -1;
  _port = port;
  for (size_t n = 0; n < CHAT_SERVER_MAXCON; n++) {
    _client[n] = -1;
  }
}

ChatServer::~ChatServer() {
  stop();
}

void ChatServer::start() {
  if (_socket >= 0) {
    Serial.println("Cannot start: Server already started.");
    return;
  }

  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket >= 0) {
    _sock_addr.sin_family = AF_INET;
    _sock_addr.sin_addr.s_addr = 0;
    _sock_addr.sin_port = htons(_port);
    int err = bind(_socket, (struct sockaddr* )&_sock_addr, sizeof(_sock_addr));
    if (!err) {
      err = listen(_socket, 3);
      if (!err) {
        return;
      }
      Serial.println("Cannot start: listen() failed.");
      close(_socket);
    }
    Serial.println("Cannot start: bind() failed.");
  }
  Serial.println("Cannot start: Cannot create socket.");
  _socket = -1;
}

void ChatServer::loop() {
  if (_socket < 0) {
    return;
  }
  for (size_t n = 0; n < CHAT_SERVER_MAXCON; n++) {
    if (_client[n] >= 0) {
      handleConnection(n);
    } else {
      handleIncoming(n);
    }
  }
}

void ChatServer::stop() {
  if (_socket!=-1) {
    for (size_t n = 0; n < CHAT_SERVER_MAXCON; n++) {
      if (_client[n] >= 0) {
        close(_client[n]);
        _client[n] = 0;
      }
    }
    close(_socket);
    _socket = -1;
  } else {
    Serial.println("Cannot stop: Server not started.");
  }
}

void ChatServer::handleIncoming(size_t freeConID) {
  timeval timeout = {.tv_sec = 0, .tv_usec = 0};
  fd_set sockfds;
  FD_ZERO(&sockfds);
  FD_SET(_socket, &sockfds);
  select(_socket + 1, &sockfds, NULL, NULL, &timeout);
  if (FD_ISSET(_socket, &sockfds)) {
    int newCon = accept(_socket, NULL, 0);
    if (newCon >= 0) {
      _client[freeConID] = newCon;
    }
  }
}

void ChatServer::handleConnection(size_t conID) {
  timeval timeout = {.tv_sec = 0, .tv_usec = 0};
  fd_set sockfds;
  FD_ZERO(&sockfds);
  FD_SET(_client[conID], &sockfds);
  select(_client[conID] + 1, &sockfds, NULL, NULL, &timeout);
  if (FD_ISSET(_client[conID], &sockfds)) {
    uint8_t buf[128];
    ssize_t len = recv(_client[conID], buf, sizeof(buf), MSG_WAITALL | MSG_DONTWAIT);
    if (len > 0) {
      for (size_t other = 0; other < CHAT_SERVER_MAXCON; other++) {
        if (other != conID) {
          send(_client[other], buf, len, 0);
        }
      }
      Serial.write(buf, len);
    } else {
      // =0 -> closed, <0 -> error
      close(_client[conID]);
      _client[conID] = -1;
    }
  }
}
