#ifndef LOGGING_H
#define LOGGING_H

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>    
#include <chrono>

#include <csignal>

#include <libraries/json/json.hpp>
using json = nlohmann::json;


#include <chrono>
#include <string>
#include <atomic>

#include <fcntl.h>
#include <unistd.h>

#include <libraries/readerwriterqueue/readerwriterqueue.h>
#include <libraries/readerwriterqueue/atomicops.h>

using namespace moodycamel;


class Log {
 public:
  Log(zmq::context_t& ctx);
  virtual ~Log();

  virtual bool open();
  virtual void close();

  virtual bool pullEvent(std::string& data);
  virtual void pushEvent(std::string  data);

  bool publish_message(std::string message);

  virtual void startProcessing();
  virtual void stopProcessing();
  void processEventLoop();

protected:
  std::mutex state_lock;

  zmq::context_t& context;
  zmq::socket_t socket_pub{context, zmq::socket_type::pub};
    
  std::string socket_pub_address {"tcp://0.0.0.0:6000"};

  BlockingReaderWriterQueue<std::string> log_queue{1000}; 

  std::atomic<bool> running{true};
  std::thread process_thread;

};

#endif