/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Charles J. Cliffe

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

#include "SoapyLoopbackRx.hpp"
#include "SoapyLoopbackTx.hpp"
#include <SoapySDR/Registry.hpp>
#include "config.h"

static std::string get_tuner(const std::string &serial, const size_t deviceIndex)
{
    return "1";
}

static std::vector<SoapySDR::Kwargs> findTx(const SoapySDR::Kwargs &args)
{
    std::vector<SoapySDR::Kwargs> results;

    SoapySDR::Kwargs devInfo;

    devInfo["label"] = "loopback_Tx";
    devInfo["product"] = "loopback_Tx";
    devInfo["serial"] = "0000-0000";
    devInfo["manufacturer"] = "drAK";
    //devInfo["pipe"] = DEFAULT_PIPE_NAME;

    results.push_back(devInfo);

    return results;
}

static std::vector<SoapySDR::Kwargs> findRx(const SoapySDR::Kwargs &args)
{
    std::vector<SoapySDR::Kwargs> results;

    SoapySDR::Kwargs devInfo;

    devInfo["label"] = "loopback_Rx";
    devInfo["product"] = "loopback_Rx";
    devInfo["serial"] = "0000-0000";
    devInfo["manufacturer"] = "drAK";
    //devInfo["pipe"] = DEFAULT_PIPE_NAME;

    results.push_back(devInfo);

    return results;
}


static SoapySDR::Device *makeTx(const SoapySDR::Kwargs &args)
{
    return new SoapyLoopbackTx(args);
}

static SoapySDR::Device *makeRx(const SoapySDR::Kwargs &args)
{
    return new SoapyLoopbackRx(args);
}

static SoapySDR::Registry registerLoopbackTx("drAK_loopbackTx", &findTx, &makeTx, SOAPY_SDR_ABI_VERSION);
static SoapySDR::Registry registerLoopbackRx("drAK_loopbackRx", &findRx, &makeRx, SOAPY_SDR_ABI_VERSION);
