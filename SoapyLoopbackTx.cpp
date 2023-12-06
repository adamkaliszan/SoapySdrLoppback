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

#include <algorithm>
#include <climits>
#include <cstring>
#include <thread>
#include <chrono>

#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>

#include "SoapyLoopbackTx.hpp"

using namespace std::chrono_literals;

SoapyLoopbackTx::SoapyLoopbackTx(const SoapySDR::Kwargs &args): SoapyLoopback(args)
{
}

SoapyLoopbackTx::~SoapyLoopbackTx(void)
{
    //cleanup device handles
    //rtlsdr_close(dev);
}

/*******************************************************************
 * Async thread work
 ******************************************************************/


/*******************************************************************
 * Stream API
 ******************************************************************/

int SoapyLoopbackTx::writeStream(
        SoapySDR::Stream *stream,
        const void * const *buffs,
        const size_t numElems,
        int &flags,
        const long long timeNs,
        const long timeoutUs)
{
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::writeStream");
    size_t handle;
    void *buff;
    int capacity = numElems;
    capacity = acquireWriteBuffer(stream, handle, &buff, timeoutUs);
    int result = std::min<int>(capacity, numElems);
    memcpy(buff, *buffs, result * stream->itemSize);
    releaseWriteBuffer(stream, handle, numElems, flags, timeNs);
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::writeStream DONE");
    return result;
}

/*******************************************************************
 * Direct buffer access API
 ******************************************************************/

int SoapyLoopbackTx::acquireWriteBuffer(
    SoapySDR::Stream *stream,
    size_t &handle,
    void **buffs,
    const long timeoutUs)
{
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::acquireWriteBuffer");
    do {
        buffer.aquired.frame = std::move(stream->pipe->pullEmpty(static_cast<std::chrono::microseconds>(timeoutUs)));
//        SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::acquireWriteBuffer frame");
    }
    while (!buffer.aquired.frame && stream->pipe->isActive());
    if (!stream->pipe->isActive()) {
        buffer.aquired.currentBuff = nullptr;
        buffer.aquired.bufferedElems = 0;
        return 0;
    }

    buffer.aquired.currentBuff = (char *) buffer.aquired.frame->data.data();
    buffs[0] = buffer.aquired.currentBuff;
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::acquireWriteBuffer DONE");
    return buffer.aquired.frame->data.size() / stream->itemSize;
}

void SoapyLoopbackTx::releaseWriteBuffer(
    SoapySDR::Stream *stream,
    const size_t handle,
    const size_t numElems,
    int &flags,
    const long long timeNs) 
{
    //SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::releaseWriteBuffer");
    buffer.aquired.frame->data.resize(numElems * stream->itemSize);
    stream->pipe->pushData(std::move(buffer.aquired.frame));
    buffer.aquired.currentBuff = nullptr;
    buffer.aquired.bufferedElems = 0;
}

int SoapyLoopbackTx::activateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems)
{
    //if (flags != 0) return SOAPY_SDR_NOT_SUPPORTED;
    SoapySDR_logf(SOAPY_SDR_DEBUG, "SoapyLoopbackTx::activateStream. Using connector %s", stream->pipeName.c_str());

    stream->pipe = Connector::getConnector(stream->pipeName);
    stream->pipe->FillEmpty(stream->noOfBuffers, stream->bufferSize);
    stream->pipe->activate();
    return numElems;
}

int SoapyLoopbackTx::deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs)
{
    SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopbackTx::deactivateStream");

    if (flags != 0) {
        SoapySDR_logf(SOAPY_SDR_ERROR, "SoapyLoopbackTx::deactivateStream %x flats not supported", flags);
        return SOAPY_SDR_NOT_SUPPORTED;
    }
    stream->pipe->notiffyExit();
    return 0;
}