/*
    Vamp Lossy Encoding Detector
    Chris Cannam, Queen Mary University of London
    Copyright (c) 2025 Queen Mary University of London

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include "QuickLossyDetector.h"

#include "detect.h"
#include "version.h"

#include <cmath>

using namespace std;

QuickLossyDetector::QuickLossyDetector(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_blockSize(512),
    m_imageWidth(172),
    m_enough(false),
    m_lastTimestamp(Vamp::RealTime::zeroTime)
{
}

QuickLossyDetector::~QuickLossyDetector()
{
}

string
QuickLossyDetector::getIdentifier() const
{
    return "quicklossydetector";
}

string
QuickLossyDetector::getName() const
{
    return "Lossy Encoding Detector (Quick)";
}

string
QuickLossyDetector::getDescription() const
{
    return "Tests and reports whether audio appears to have been previously compressed using a lossy encoder such as MP3. Unlike the full version of the plugin, this one examines only a tiny part of the input.";
}

string
QuickLossyDetector::getMaker() const
{
    return "Chris Cannam";
}

int
QuickLossyDetector::getPluginVersion() const
{
    return PLUGIN_VERSION_INT;
}

string
QuickLossyDetector::getCopyright() const
{
    return "MIT/X11 licence";
}

QuickLossyDetector::InputDomain
QuickLossyDetector::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
QuickLossyDetector::getPreferredBlockSize() const
{
    return m_blockSize;
}

size_t 
QuickLossyDetector::getPreferredStepSize() const
{
    return m_blockSize / 2;
}

size_t
QuickLossyDetector::getMinChannelCount() const
{
    return 1;
}

size_t
QuickLossyDetector::getMaxChannelCount() const
{
    return 1;
}

QuickLossyDetector::ParameterList
QuickLossyDetector::getParameterDescriptors() const
{
    ParameterList list;
    return list;
}

float
QuickLossyDetector::getParameter(string) const
{
    return 0;
}

void
QuickLossyDetector::setParameter(string, float) 
{
}

QuickLossyDetector::ProgramList
QuickLossyDetector::getPrograms() const
{
    ProgramList list;
    return list;
}

string
QuickLossyDetector::getCurrentProgram() const
{
    return ""; // no programs
}

void
QuickLossyDetector::selectProgram(string)
{
}

QuickLossyDetector::OutputList
QuickLossyDetector::getOutputDescriptors() const
{
    OutputList list;

    int outputNo = 0;

    OutputDescriptor d;
    d.identifier = "lossy";
    d.name = "QuickLossy";
    d.description = "A single estimate for whether the input has been lossily encoded in the past or not. 1 indicates yes it has, 0 indicates no it hasn't.";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = true;
    d.minValue = 0.f;
    d.maxValue = 1.f;
    d.isQuantized = true;
    d.quantizeStep = 1.f;
    d.sampleType = OutputDescriptor::VariableSampleRate;
    d.hasDuration = false;
    m_lossyOutput = outputNo++;
    list.push_back(d);

    return list;
}

bool
QuickLossyDetector::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) {
        std::cerr << "QuickLossyDetector::initialise: unsupported channel count "
                  << channels << std::endl;
        return false;
    }

    if (blockSize != getPreferredBlockSize()) {
        std::cerr << "QuickLossyDetector::initialise: block size " << blockSize
                  << " must match requested block size " << getPreferredBlockSize()
                  << std::endl;
        return false;
    }

    if (stepSize != getPreferredStepSize()) {
        std::cerr << "QuickLossyDetector::initialise: step size " << stepSize
                  << " must match requested step size " << getPreferredStepSize()
                  << std::endl;
        return false;
    }

    return true;
}

void
QuickLossyDetector::reset()
{
    m_buildingImage = {};
    m_enough = false;
    m_lastTimestamp = Vamp::RealTime::zeroTime;
}

QuickLossyDetector::FeatureSet
QuickLossyDetector::process(const float *const *inputBuffers,
                            Vamp::RealTime timestamp)
{
    m_lastTimestamp = timestamp;
    
    if (m_enough) {
        return {};
    }
    
    int height = m_blockSize / 2 + 1;
    float scale = 1.f / sqrtf(float(m_blockSize));
    t_1 column(height, 0.f);

    for (int i = 0; i < height; ++i) {
        int ix = height - i - 1; // Image is "upside-down"
        float re = inputBuffers[0][i*2];
        float im = inputBuffers[0][i*2+1];
        float mag = sqrtf(re*re + im*im);
        mag *= scale;
        if (mag == 0.0) {
            column[ix] = 0.f;
        } else {
            float db = 20.f * log10f(mag);
            float level = 1.f + (db / 120.f);
            if (level < 0.f) level = 0.f;
            if (level > 1.f) level = 1.f;
            column[ix] = level;
        }
    }
    
    m_buildingImage.push_back(column);

    int w = int(m_buildingImage.size());

    if (w < m_imageWidth) {
        return {};
    } else if (w > m_imageWidth) {
        m_buildingImage.pop_front();
    }
    
    Vamp::RealTime rtWidth = Vamp::RealTime::frame2RealTime
        ((m_imageWidth - 1) * (m_blockSize/2), m_inputSampleRate);
    
    if (timestamp >= Vamp::RealTime::fromSeconds(30) + rtWidth) {
        m_enough = true;
    }

    return {};
}

QuickLossyDetector::FeatureSet
QuickLossyDetector::getRemainingFeatures()
{
    if (int(m_buildingImage.size()) < m_imageWidth) {
        return {};
    }

    t_2 image;
    for (int i = 0; i < m_imageWidth; ++i) {
        image.push_back(m_buildingImage[i]);
    }
    
    t_1 result = classify(image);

    Feature f;

    f.hasTimestamp = true;
    f.timestamp = Vamp::RealTime::zeroTime;
    f.hasDuration = true;
    f.duration = m_lastTimestamp;

    if (result[0] < 0.5f) {
        f.values.push_back(0.f);
        f.label = "Original";
    } else {
        f.values.push_back(1.f);
        f.label = "Lossy";
    }
    
    FeatureSet fs;
    fs[m_lossyOutput].push_back(f);
    return fs;
}

