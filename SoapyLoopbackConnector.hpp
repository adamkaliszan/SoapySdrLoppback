#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <SoapySDR/Logger.hpp>

struct Frame
{
    unsigned long long tick;
    std::vector<signed char> data;

    Frame(size_t size) {
      data.resize(size);
    }
};

class Connector {
  private:
    std::queue<std::unique_ptr<Frame>> tx2rx;
    std::queue<std::unique_ptr<Frame>> rx2tx;
    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<bool> doWork{true};

  public:
    void FillEmpty(int noOfBuffers, size_t bufferSize);

    void pushData(std::unique_ptr<Frame> &&frame);
    void pushEmpty(std::unique_ptr<Frame> &&frame);

    void activate();
    bool isActive() { return doWork; }
    void notiffyExit();

    std::unique_ptr<Frame> pullEmpty(std::chrono::microseconds duration);
    std::unique_ptr<Frame> pullData(std::chrono::microseconds duration);

    static std::map<std::string, std::shared_ptr<Connector>> connectors;
    static std::shared_ptr<Connector> getConnector(std::string name);

    static std::mutex cr_mutex;
};


namespace SoapySDR {

    class Stream {
      public:
        Stream();
        std::shared_ptr<Connector> pipe {};
        int itemSize {0};
        int bufferSize {0};
        int noOfBuffers {0};
        std::string pipeName{"default"};
    };
}