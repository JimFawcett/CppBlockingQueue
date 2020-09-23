///////////////////////////////////////////////////////////////
// Cpp11-BlockingQueue.cpp - Thread-safe Blocking Queue      //
// ver 1.4                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2013 //
///////////////////////////////////////////////////////////////

#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <string>
#include <iostream>
#include <sstream>
#include "Cpp11-BlockingQueue.h"

#ifdef TEST_BLOCKINGQUEUE

std::mutex ioLock;

void test(BlockingQueue<std::string>* pQ)
{
  std::string msg;
  do
  {
    msg = pQ->deQ();
    {
      std::lock_guard<std::mutex> l(ioLock);
      std::cout << "\n  thread deQed " << msg.c_str();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } while(msg != "quit");
}

class Msg {
public:
  Msg(const std::string& cnts = "") : contents(cnts) {

  }
  Msg(const Msg& msg) : contents(msg.contents) {
    std::cout << "\n  copied Msg";
  }
  Msg(Msg&& msg) : contents(std::move(msg.contents)) {
    std::cout << "\n  moved Msg";
  }
  std::string& msg_contents() { return contents; }
private:
  std::string contents;
};

int main()
{
  std::cout << "\n  Demonstrating C++11 Blocking Queue";
  std::cout << "\n ====================================\n";

  std::cout << "\n  -- test enQ and deQ operations --";

  BlockingQueue<Msg> mQ;
  Msg tmsg("a message");
  std::cout << "\n  enqueuing " << "\"" << tmsg.msg_contents() << "\"";
  mQ.enQ(tmsg);
  auto dm = mQ.deQ();
  std::cout << "\n  dequeued " << "\"" << dm.msg_contents() << "\"\n";

  std::cout << "\n  -- test inter-thread messaging --";
  BlockingQueue<std::string> q;
  std::thread t(test, &q);

  for(int i=0; i<15; ++i)
  {
    std::ostringstream temp;
    temp << i;
    std::string msg = std::string("msg#") + temp.str();
    {
      std::lock_guard<std::mutex> l(ioLock);
      std::cout << "\n   main enQing " << msg.c_str();
    }
    q.enQ(msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
  q.enQ("quit");
  t.join();

  std::cout << "\n";
  std::cout << "\n  Move construction of BlockingQueue";
  std::cout << "\n ------------------------------------";

  std::string msg = "test";
  q.enQ(msg);
  std::cout << "\n  before move:";
  std::cout << "\n    q.size() = " << q.size();
  std::cout << "\n    q.front() = " << q.front();
  BlockingQueue<std::string> q2 = std::move(q);  // move assignment
  std::cout << "\n  after move:";
  std::cout << "\n    q2.size() = " << q2.size();
  std::cout << "\n    q.size() = " << q.size();
  std::cout << "\n    q2 element = " << q2.deQ() << "\n";

  std::cout << "\n  Move assigning state of BlockingQueue";
  std::cout << "\n ---------------------------------------";
  BlockingQueue<std::string> q3;
  q.enQ("test");
  std::cout << "\n  before move:";
  std::cout << "\n    q.size() = " << q.size();
  std::cout << "\n    q.front() = " << q.front();
  q3 = std::move(q);
  std::cout << "\n  after move:";
  std::cout << "\n    q.size() = " << q.size();
  std::cout << "\n    q3.size() = " << q3.size();
  std::cout << "\n    q3 element = " << q3.deQ() << "\n";

  std::cout << "\n\n";
}

#endif
