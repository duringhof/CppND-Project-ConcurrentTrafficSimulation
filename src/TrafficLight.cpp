#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {

  // Done.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait() to wait for and receive new messages and pull them from
  // the queue using move semantics. The received object should then be returned
  // by the receive function.

  // perform queue modification under the lock
  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

  // remove last vector element from queue
  T msg = std::move(_queue.back());
  _queue.pop_back();

  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {

  // Done.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex> as well as _condition.notify_one() to add a new
  // message to the queue and afterwards send a notification.

  // perform vector modification under the lock
  std::lock_guard<std::mutex> uLock(_mutex);

  // add vector to queue & notify client after pushing new Vehicle into vector
  _queue.push_back(std::move(msg));
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

/*
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an
infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

*/

void TrafficLight::simulate() {

  // Done.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.

  //Start cycleThroughPhases in a thread that is added to the thread queue
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {

  // Done.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.

  // Cycle time randomizer
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(4000, 6000);
  long cycleTime = dist(mt);

  // Initialize stopwatch
  auto t_0 = std::chrono::steady_clock::now();

  while (true) {

    // Measure time between loop cycles
    long t_delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - t_0)
                       .count();

    if (t_delta >= cycleTime) {

      // Toggle between red and green
      if (_currentPhase == TrafficLightPhase::kGreen)
        _currentPhase = TrafficLightPhase::kRed;
      else if (_currentPhase == TrafficLightPhase::kRed)
        _currentPhase = TrafficLightPhase::kGreen;

      // Send update method to the message queue using move semantics
      _queue.send(std::move(_currentPhase));

      // Reset stopwatch and renew randomized cycle time
      t_0 = std::chrono::steady_clock::now();
      cycleTime = dist(mt);
    }

    // Sleep 1 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}