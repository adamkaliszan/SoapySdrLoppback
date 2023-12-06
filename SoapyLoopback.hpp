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

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>
#include <atomic>

#include "SoapyLoopbackConnector.hpp"

class SoapyLoopback: public SoapySDR::Device
{
public:
    SoapyLoopback(const SoapySDR::Kwargs &args);

    ~SoapyLoopback(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const override;
    SoapySDR::Kwargs getHardwareInfo(void) const override;


    /*******************************************************************
     * Channels API
     ******************************************************************/

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/

    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels =
            std::vector<size_t>(), const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream *stream);

    size_t getStreamMTU(SoapySDR::Stream *stream) const;

    /*******************************************************************
     * Direct buffer access API
     ******************************************************************/

    size_t getNumDirectAccessBuffers(SoapySDR::Stream *stream);

    int getDirectAccessBufferAddrs(SoapySDR::Stream *stream, const size_t handle, void **buffs);

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/

    bool hasDCOffsetMode(const int direction, const size_t channel) const;

    bool hasFrequencyCorrection(const int direction, const size_t channel) const;

    void setFrequencyCorrection(const int direction, const size_t channel, const double value);

    double getFrequencyCorrection(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/

    std::vector<std::string> listGains(const int direction, const size_t channel) const;

    bool hasGainMode(const int direction, const size_t channel) const;

    void setGainMode(const int direction, const size_t channel, const bool automatic);

    bool getGainMode(const int direction, const size_t channel) const;

    void setGain(const int direction, const size_t channel, const double value);

    void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    double getGain(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/

    void setFrequency(
            const int direction,
            const size_t channel,
            const std::string &name,
            const double frequency,
            const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;

    /*******************************************************************
     * Sample Rate API
     ******************************************************************/

    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

    void setBandwidth(const int direction, const size_t channel, const double bw);

    double getBandwidth(const int direction, const size_t channel) const;

    std::vector<double> listBandwidths(const int direction, const size_t channel) const;

    SoapySDR::RangeList getBandwidthRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Time API
     ******************************************************************/

    std::vector<std::string> listTimeSources(void) const;

    std::string getTimeSource(void) const;

    void setTimeSource(const std::string &what = "");

    bool hasHardwareTime(const std::string &what = "") const;

    long long getHardwareTime(const std::string &what = "") const;

    void setHardwareTime(const long long timeNs, const std::string &what = "");

 /*******************************************************************
     * Clocking API
     ******************************************************************/

    void setMasterClockRate(const double rate);

    double getMasterClockRate(void) const;

    SoapySDR::RangeList getMasterClockRates(void) const;

    std::vector<std::string> listClockSources(void) const;

    void setClockSource(const std::string &source);

    std::string getClockSource(void) const;

    /*******************************************************************
     * Sensor API
     ******************************************************************/

    std::vector<std::string> listSensors(void) const;

    SoapySDR::ArgInfo getSensorInfo(const std::string &name) const;

    std::string readSensor(const std::string &name) const;

    std::vector<std::string> listSensors(const int direction, const size_t channel) const;

    SoapySDR::ArgInfo getSensorInfo(const int direction, const size_t channel, const std::string &name) const;

    std::string readSensor(const int direction, const size_t channel, const std::string &name) const;

    /*******************************************************************
     * Settings API
     ******************************************************************/

    SoapySDR::ArgInfoList getSettingInfo(void) const;

    void writeSetting(const std::string &key, const std::string &value);

    std::string readSetting(const std::string &key) const;

protected:
    SoapySDR::Stream stream;

    // clock API
    std::string _ref_source;

    std::string time_source;

    //int tunerType;
    uint32_t sampleRate, centerFrequency, bandwidth;
    double ppm, directSamplingMode;
    size_t numBuffers, bufferLength, asyncBuffs;
    bool iqSwap, gainMode, offsetMode, digitalAGC, biasTee;
    double IFGain[6], tunerGain;
    std::atomic<long long> ticks;

  protected:
    struct {    
        struct {
            std::unique_ptr<Frame> frame;
            size_t bufferedElems;
            char *currentBuff;
        } aquired { {}, 0, nullptr};
        
        std::atomic<bool> reset;
        std::atomic<bool> overflow;
        long long ticks;
    } buffer;

    double gainMin, gainMax;
};
