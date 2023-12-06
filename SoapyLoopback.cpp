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
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Time.hpp>
#include <algorithm>
#include <climits>
#include <cstring>
#include <string>

#include "config.h"

std::vector<std::string> SoapyLoopback::getStreamFormats(const int direction, const size_t channel) const {
    std::vector<std::string> formats;

    formats.push_back(SOAPY_SDR_CS8);
    formats.push_back(SOAPY_SDR_CS12);
    formats.push_back(SOAPY_SDR_CS16);
    formats.push_back(SOAPY_SDR_CF32);

    return formats;
}

std::string SoapyLoopback::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const {

     return SOAPY_SDR_CS12;
}

SoapySDR::ArgInfoList SoapyLoopback::getStreamArgsInfo(const int direction, const size_t channel) const {
    SoapySDR::ArgInfoList streamArgs;

    SoapySDR::ArgInfo bufflenArg;
    bufflenArg.key = "bufflen";
    bufflenArg.value = std::to_string(DEFAULT_BUFFER_LENGTH);
    bufflenArg.name = "Buffer Size";
    bufflenArg.description = "Number of bytes per buffer, multiples of 512 only.";
    bufflenArg.units = "bytes";
    bufflenArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(bufflenArg);

    SoapySDR::ArgInfo buffersArg;
    buffersArg.key = "buffers";
    buffersArg.value = std::to_string(DEFAULT_NUM_BUFFERS);
    buffersArg.name = "Ring buffers";
    buffersArg.description = "Number of buffers in the ring.";
    buffersArg.units = "buffers";
    buffersArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(buffersArg);

    SoapySDR::ArgInfo asyncbuffsArg;
    asyncbuffsArg.key = "pipe";
    asyncbuffsArg.value = DEFAULT_PIPE_NAME;
    asyncbuffsArg.name = "Pipe name";
    asyncbuffsArg.description = "Pipe (Tx -> Rx) name. There are many pairs Tx Rx";
    asyncbuffsArg.units = "buffers";
    asyncbuffsArg.type = SoapySDR::ArgInfo::STRING;

    streamArgs.push_back(asyncbuffsArg);

    return streamArgs;
}

/*******************************************************************
 * Stream API
 ******************************************************************/

SoapySDR::Stream *SoapyLoopback::setupStream(
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args)
{
    SoapySDR::Stream &result = stream;

    SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopback::setupStream");

    for (const auto &[key, value]: args) {
        SoapySDR_logf(SOAPY_SDR_INFO, "%s: %s", key.c_str(), value.c_str());
    }

    result.bufferSize = (args.count("buffers") > 0) ? std::stoi(args.at("bufflen")) : DEFAULT_BUFFER_LENGTH;
    result.noOfBuffers = (args.count("buffers") > 0) ? std::stoi(args.at("buffers")) : DEFAULT_NUM_BUFFERS;
    result.pipeName = (args.count("pipe") > 0) ? args.at("pipe") : DEFAULT_PIPE_NAME;

    //check the channel configuration
    if (channels.size() > 1 or (channels.size() > 0 and channels.at(0) != 0))
    {
        throw std::runtime_error("setupStream invalid channel selection");
    }

    //check the format
    if (format == SOAPY_SDR_CF32)
    {
        SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopback: Using format CF32.");
        result.itemSize = 8;
        //rxFormat = RTL_RX_FORMAT_FLOAT32;
    }
    else if (format == SOAPY_SDR_CS12)
    {
        SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopback: setupStream Using format CS12.");
        result.itemSize = 3;
        //rxFormat = RTL_RX_FORMAT_INT16;
    }
    else if (format == SOAPY_SDR_CS16)
    {
        SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopback: Using format CS16.");
        result.itemSize = 4;
        //rxFormat = RTL_RX_FORMAT_INT16;
    }
    else if (format == SOAPY_SDR_CS8) {
        SoapySDR_log(SOAPY_SDR_INFO, "SoapyLoopback: Using format CS8.");
        result.itemSize = 2;
        //rxFormat = RTL_RX_FORMAT_INT8;
    }
    else
    {
        throw std::runtime_error(
                "setupStream invalid format '" + format
                        + "' -- Only CS8, CS16 and CF32 are supported by SoapyLoopback module.");
    }
    SoapySDR_logf(SOAPY_SDR_INFO, "Loopback Using buffer length %d, %d buffers, item size = %d", result.bufferSize, result.noOfBuffers, result.itemSize);

//allocate buffers postphoned till stream activation
    return &stream;
}

void SoapyLoopback::closeStream(SoapySDR::Stream *stream)
{

}

size_t SoapyLoopback::getStreamMTU(SoapySDR::Stream *stream) const
{
    return bufferLength / BYTES_PER_SAMPLE;
}

/*******************************************************************
 * Direct buffer access API
 ******************************************************************/

size_t SoapyLoopback::getNumDirectAccessBuffers(SoapySDR::Stream *stream)
{
    return 1;
}

int SoapyLoopback::getDirectAccessBufferAddrs(SoapySDR::Stream *stream, const size_t handle, void **buffs)
{
    if (buffer.aquired.frame)
        buffs[0] = (void *)buffer.aquired.frame->data.data();
    else
        buffs[0] = nullptr;
    return 0;
}
