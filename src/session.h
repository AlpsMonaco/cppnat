#ifndef __CPPNAT_SESSION_H__
#define __CPPNAT_SESSION_H__

#include "message.h"

NAMESPACE_CPPNAT_START

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(SocketPtr, const MessageHandler &, const ErrorHandler &error_handler);
  ~Session();

  MessageWriter GetMessageWriter();

  void Start();

 protected:
  static constexpr size_t buffer_size_ = Protocol::max_size;

  SocketPtr socket_ptr_;
  size_t read_size_;
  size_t extra_offset_;
  char buffer_[buffer_size_];
  const MessageHandler &message_handler_;
  ErrorHandler error_handler_;

  void MessageLoop();
  void ParseBuffer();
  void HandleBuffer(const Protocol::Header &, const char *);
  void ParseExtraBuffer();
};

NAMESPACE_CPPNAT_END

#endif