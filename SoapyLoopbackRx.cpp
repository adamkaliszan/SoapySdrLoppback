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

#include "SoapyLoopback.hpp"

#include <chrono>
#include <cstring>

#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>

#include "SoapyLoopbackRx.hpp"
#include "config.h"

using namespace std::chrono_literals;

SoapyLoopbackRx::SoapyLoopbackRx(const SoapySDR::Kwargs &args): SoapyLoopback(args)
{

}

SoapyLoopbackRx::~SoapyLoopbackRx(void)
{
    //cleanup device handles
    //rtlsdr_close(dev);
}

/*******************************************************************
 * Stream API
 ******************************************************************/

int SoapyLoopbackRx::readStream(
        SoapySDR::Stream *stream,
        void * const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs)
{
    //this is the user's buffer for channel 0
    void *buff0 = buffs[0];

    //drop remainder buffer on reset
    if (buffer.reset && buffer.aquired.bufferedElems != 0)
    {
        buffer.aquired.bufferedElems = 0;
        if (buffer.aquired.frame)
            this->releaseReadBuffer(stream, 0);
    }

    //are elements left in the buffer?
    if (buffer.aquired.bufferedElems == 0)
    {   //if not, do a new read.
        size_t handle = 0;
        int ret = this->acquireReadBuffer(stream, handle, (const void **)&buffer.aquired.currentBuff, flags, timeNs, timeoutUs);
        if (ret <= 0) 
            return ret;
        buffer.aquired.bufferedElems = ret;
    }
    else
    {   //otherwise just update return time to the current tick count
        //flags |= SOAPY_SDR_HAS_TIME;
        //timeNs = SoapySDR::ticksToTimeNs(bufTicks, sampleRate);
    }

    size_t returnedElems = std::min(buffer.aquired.bufferedElems, numElems);


    memcpy(buff0, buffer.aquired.currentBuff, returnedElems*stream->itemSize);
    //bump variables for next call into readStream
    buffer.aquired.bufferedElems -= returnedElems;
    buffer.aquired.currentBuff += returnedElems*stream->itemSize;
    //bufTicks += returnedElems; //for the next call to readStream if there is a remainder

    //return number of elements written to buff0
    if (buffer.aquired.bufferedElems > 0)
        flags |= SOAPY_SDR_MORE_FRAGMENTS;
    else 
        this->releaseReadBuffer(stream, 0);

    return returnedElems;
}

/*******************************************************************
 * Direct buffer access API
 ******************************************************************/


int SoapyLoopbackRx::acquireReadBuffer(
    SoapySDR::Stream *stream,
    size_t &handle,
    const void **buffs,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackRx::acquireReadBuffer");
    do {
      buffer.aquired.frame = std::move(stream->pipe->pullData(static_cast<std::chrono::microseconds>(timeoutUs)));
    }
    while (stream->pipe->isActive() && !buffer.aquired.frame);

    if (!buffer.aquired.frame) {
        buffer.aquired.currentBuff = nullptr;
        buffer.aquired.bufferedElems = 0;
        return 0;
    }

    buffer.aquired.currentBuff = (char *) buffer.aquired.frame->data.data();
    buffs[0] = buffer.aquired.frame->data.data();
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackRx::acquireReadBuffer DONE");
    return buffer.aquired.frame->data.size() / stream->itemSize;
}

void SoapyLoopbackRx::releaseReadBuffer(
    SoapySDR::Stream *stream,
    const size_t handle) 
{
    stream->pipe->pushEmpty(std::move(buffer.aquired.frame));
    buffer.aquired.currentBuff = nullptr;
    buffer.aquired.bufferedElems = 0;
}

int SoapyLoopbackRx::activateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems)
{
    if (flags != 0) 
        return SOAPY_SDR_NOT_SUPPORTED;

    SoapySDR_logf(SOAPY_SDR_INFO, "SoapyLoopbackRx::activateStream with %d elems", numElems);

    //start the async thread

    stream->pipe = Connector::getConnector(stream->pipeName);
    stream->pipe->activate();
    return numElems;
}

int SoapyLoopbackRx::deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs)
{
    SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackRx::deactivateStream");

    if (flags != 0) 
        return SOAPY_SDR_NOT_SUPPORTED;

    stream->pipe->notiffyExit();
    return 0;
}
