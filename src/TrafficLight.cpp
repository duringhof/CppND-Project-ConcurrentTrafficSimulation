#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T> T MessageQueue<T>::receive() {

  // perform queue modification under the lock
  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

  // remove last vector element from queue
  T msg = std::move(_queue.back());
  _queue.pop_back();

  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {

  // perform vector modification under the lock
  std::lock_guard<std::mutex> uLock(_mutex);

  // add vector to queue & notify client after pushing new Vehicle into vector
  _queue.push_back(std::move(msg));
  _cond.notify_one();
}

TrafficLight::TrafficLight() {

  _currentPhase = TrafficLightPhase::kRed;
}

void TrafficLight::waitForGreen() {

  while (true) {

    TrafficLightPhase message = _queue.receive();
    if (message == TrafficLightPhase::kGreen)
      return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() {

  return _currentPhase;
}

void TrafficLight::simulate() {

  //Start cycleThroughPhases in a thread that is added to the thread queue
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {

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