#ifndef CPP11_BLOCKINGQUEUE_H
#define CPP11_BLOCKINGQUEUE_H
///////////////////////////////////////////////////////////////
// Cpp11-BlockingQueue.h - Thread-safe Blocking Queue        //
// ver 1.5 - 25 Sep 2020                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015 //
///////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package contains one thread-safe class: BlockingQueue<T>.
 * Its purpose is to support sending messages between threads.
 * It is implemented using C++11 threading constructs including 
 * std::condition_variable and std::mutex.  The underlying storage
 * is provided by the non-thread-safe std::queue<T>.
 * 
 * Note: 
 * The enQ(T t) accepts its argument by value.  If you pass
 * a reference a second enQ could change the contents of the
 * enqueued first message.  Whether that happens is time
 * dependent, e.g., a data race.  So we avoid that at the
 * expense of a message copy.
 * 
 * It is desireable to provide move semantics for the type
 * stored in this queue.  That avoids multiple copies.
 * A single enQ and deQ results in one copy and three moves.
 *
 * You can see where those occur by looking at the enQ and
 * deQ implementations, below.
 * 
 * Required Files:
 * ---------------
 * Cpp11-BlockingQueue.h
 *
 * Build Process:
 * --------------
 * devenv Cpp11-BlockingQueue.sln /rebuild debug
 *
 * Maintenance History:
 * --------------------
 * ver 1.5 : 25 Sep 2020
 * - fixed omission of move in wait loop
 * ver 1.4 : 23 Sep 2020
 * - change enQ(T t) to pass argument T by value
 * ver 1.3 : 04 Mar 2016
 * - changed behavior of front() to throw exception
 *   on empty queue.
 * - added comment about std::unique_lock in deQ()
 * ver 1.2 : 27 Feb 2016
 * - added front();
 * - added move ctor and move assignment
 * - deleted copy ctor and copy assignment
 * ver 1.1 : 26 Jan 2015
 * - added copy constructor and assignment operator
 * ver 1.0 : 03 Mar 2014
 * - first release
 *
 */

#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <string>
#include <iostream>
#include <sstream>

template <typename T>
class BlockingQueue {
public:
  BlockingQueue() {}
  BlockingQueue(BlockingQueue<T>&& bq) noexcept;
  BlockingQueue<T>& operator=(BlockingQueue<T>&& bq) noexcept;
  BlockingQueue(const BlockingQueue<T>&) = delete;
  BlockingQueue<T>& operator=(const BlockingQueue<T>&) = delete;
  T deQ();
  void enQ(T t);
  T& front();
  void clear();
  size_t size();
private:
  std::queue<T> q_;
  std::mutex mtx_;
  std::condition_variable cv_;
};
//----< move constructor >---------------------------------------------

template<typename T>
BlockingQueue<T>::BlockingQueue(BlockingQueue<T>&& bq) noexcept
// need to lock so can't initialize
{
  std::lock_guard<std::mutex> l(mtx_);
  q_ = bq.q_;
  while (bq.q_.size() > 0)  // clear bq
    bq.q_.pop();
  /* can't copy  or move mutex or condition variable, so use default members */
}
//----< move assignment >----------------------------------------------

template<typename T>
BlockingQueue<T>& BlockingQueue<T>::operator=(BlockingQueue<T>&& bq) noexcept
{
  if (this == &bq) return *this;
  std::lock_guard<std::mutex> l(mtx_);
  q_ = bq.q_;
  while (bq.q_.size() > 0)  // clear bq
    bq.q_.pop();
  /* can't move assign mutex or condition variable so use target's */
  return *this;
}
//----< remove element from front of queue >---------------------------

template<typename T>
T BlockingQueue<T>::deQ()
{
  std::unique_lock<std::mutex> l(mtx_);
  /* 
     This lock type is required for use with condition variables.
     The operating system needs to lock and unlock the mutex:
     - when wait is called, below, the OS suspends waiting thread
       and releases lock.
     - when notify is called in enQ() the OS relocks the mutex, 
       resumes the waiting thread and sets the condition variable to
       signaled state.
     std::lock_quard does not have public lock and unlock functions.
   */
  if(q_.size() > 0)
  {
    T temp = std::move(q_.front());  // move avoids copy into temp
    q_.pop();
    return temp;
  }
  // may have spurious returns so loop on !condition

  while (q_.size() == 0)
    cv_.wait(l, [this] () { return q_.size() > 0; });
  T temp = std::move(q_.front());
  q_.pop();
  return temp;
}
//----< push element onto back of queue >------------------------------

template<typename T>
void BlockingQueue<T>::enQ(T t)
{
  {
    std::unique_lock<std::mutex> l(mtx_);
    q_.push(std::move(t));  // moves copy made on entry
  }
  cv_.notify_one();
}
//----< peek at next item to be popped >-------------------------------

template <typename T>
T& BlockingQueue<T>::front()
{
  std::lock_guard<std::mutex> l(mtx_);
  if(q_.size() > 0)
    return q_.front();
  throw std::exception("attempt to access empty queue");
}
//----< remove all elements from queue >-------------------------------

template <typename T>
void BlockingQueue<T>::clear()
{
  std::lock_guard<std::mutex> l(mtx_);
  while (q_.size() > 0)
    q_.pop();
}
//----< return number of elements in queue >---------------------------

template<typename T>
size_t BlockingQueue<T>::size()
{
  std::lock_guard<std::mutex> l(mtx_);
  return q_.size();
}

#endif
