#include "SoapyLoopbackConnector.hpp"

#include <mutex>

#include <fmt/core.h>
#include <condition_variable>

#include <SoapySDR/Logger.hpp>

using namespace std::chrono_literals;

void Connector::pushData(std::unique_ptr<Frame> &&frame) {
    //SoapySDR_log(SOAPY_SDR_INFO, "pushToForward");
    std::unique_lock lock(mutex);
    tx2rx.push(std::move(frame));
    cond.notify_all();
}

void Connector::pushEmpty(std::unique_ptr<Frame> &&frame) {
    //SoapySDR_log(SOAPY_SDR_INFO, "pushToReuse");
    std::unique_lock lock(mutex);
    rx2tx.push(std::move(frame));
    cond.notify_all();
}

void Connector::activate() {
    doWork = true;
}

void Connector::notiffyExit() {
    doWork = false;
    cond.notify_all();
}

std::unique_ptr<Frame> Connector::pullEmpty(std::chrono::microseconds duration) {
    std::unique_lock lock(mutex);

    if (rx2tx.empty()) {
        cond.wait_for(lock, duration);
    }

    if (rx2tx.empty()) {
        SoapySDR_logf(SOAPY_SDR_DEBUG, "Connector::pullEmpty FAILED, is the receiver working???");
        return {};
    }

    std::unique_ptr<Frame> result = std::move(rx2tx.front());
    //SoapySDR_logf(SOAPY_SDR_INFO, "pullEmptyFrame rx_size = %d * %d", rx2tx.size(), result->data.size());    
    rx2tx.pop();
    return result;
}

std::unique_ptr<Frame> Connector::pullData(std::chrono::microseconds duration) {
    std::unique_lock lock(mutex);
    if (doWork && tx2rx.empty()) {
        cond.wait_for(lock, duration);
    }
    if (!doWork || tx2rx.empty()) {
        return {};
    }
    std::unique_ptr<Frame> result = std::move(tx2rx.front());
    //SoapySDR_logf(SOAPY_SDR_INFO, "pullRxData tx_size = %d * %d", tx2rx.size(), result->data.size());    
    tx2rx.pop();
    return result;
}

std::map<std::string, std::shared_ptr<Connector>> Connector::connectors;
std::mutex Connector::cr_mutex;

std::shared_ptr<Connector> Connector::getConnector(std::string name) {
    std::unique_lock lock(cr_mutex);
    if (connectors.count(name) == 0) {
        SoapySDR_logf(SOAPY_SDR_INFO, "create connector \"%s\"", name.c_str());
        connectors[name] = std::make_shared<Connector>();
    } else {
        SoapySDR_logf(SOAPY_SDR_INFO, "Using existing connector \"%s\"", name.c_str());
    }
    return connectors[name];
}

void Connector::FillEmpty(int noOfBuffers, size_t bufferSize) {
    SoapySDR_logf(SOAPY_SDR_INFO, "Connector::FillEmpty(%d, %d)", noOfBuffers, bufferSize);
    while (rx2tx.size() > 0) {
        rx2tx.pop();
    }
    while (noOfBuffers-- > 0) {
        rx2tx.push(std::move(std::make_unique<Frame>(bufferSize)));
    }
}

namespace SoapySDR {
    Stream::Stream() {
        SoapySDR_logf(SOAPY_SDR_INFO, "Stream::Stream()");
    }
}
