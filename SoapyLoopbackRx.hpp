/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Charles J. Cliffe
 * Copyright (c) 2015-2017 Josh Blum

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>

#include "SoapyLoopback.hpp"
#include "config.h"

class SoapyLoopbackRx: public SoapyLoopback
{
public:
    SoapyLoopbackRx(const SoapySDR::Kwargs &args);

    ~SoapyLoopbackRx(void);


    int readStream(
            SoapySDR::Stream *stream,
            void * const *buffs,
            const size_t numElems,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000);


    /*******************************************************************
     * Direct buffer access API
     ******************************************************************/

    int acquireReadBuffer(
        SoapySDR::Stream *stream,
        size_t &handle,
        const void **buffs,
        int &flags,
        long long &timeNs,
        const long timeoutUs = 100000) override;

    void releaseReadBuffer(
        SoapySDR::Stream *stream,
        const size_t handle) override;

    int activateStream(SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems) override;

    int deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs) override;

    /*******************************************************************
     * Identification API
     ******************************************************************/
    std::string getHardwareKey(void) const override { return "drAK_LoopbackRx"; }

    /*******************************************************************
     * Antenna API
     ******************************************************************/
    std::vector<std::string> listAntennas(const int direction, const size_t channel) const override { return std::vector<std::string> {"RX"}; }
    void setAntenna(const int direction, const size_t channel, const std::string &name) override {};
    std::string getAntenna(const int direction, const size_t channel) const override { return direction ? "RX" : ""; }

    size_t getNumChannels(const int dir) const { return dir == SOAPY_SDR_RX ? NUM_CHANNELS : 0;}
    
private:
    void rx_async_operation(void);
};
