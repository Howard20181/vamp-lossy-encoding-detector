
#include "LossyDetector.h"

#include "detect.h"
#include "version.h"

#include <cmath>

using namespace std;

LossyDetector::LossyDetector(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_blockSize(512),
    m_imageWidth(172),
    m_lossyCount(0),
    m_totalCount(0)
{
}

LossyDetector::~LossyDetector()
{
}

string
LossyDetector::getIdentifier() const
{
    return "lossydetector";
}

string
LossyDetector::getName() const
{
    return "Lossy Encoding Detector";
}

string
LossyDetector::getDescription() const
{
    return "Tests and reports whether audio appears to have been previously compressed using a lossy encoder such as MP3.";
}

string
LossyDetector::getMaker() const
{
    return "Chris Cannam";
}

int
LossyDetector::getPluginVersion() const
{
    return PLUGIN_VERSION_INT;
}

string
LossyDetector::getCopyright() const
{
    return "MIT/X11 licence";
}

LossyDetector::InputDomain
LossyDetector::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
LossyDetector::getPreferredBlockSize() const
{
    return m_blockSize;
}

size_t 
LossyDetector::getPreferredStepSize() const
{
    return m_blockSize / 2;
}

size_t
LossyDetector::getMinChannelCount() const
{
    return 1;
}

size_t
LossyDetector::getMaxChannelCount() const
{
    return 1;
}

LossyDetector::ParameterList
LossyDetector::getParameterDescriptors() const
{
    ParameterList list;
    return list;
}

float
LossyDetector::getParameter(string identifier) const
{
    return 0;
}

void
LossyDetector::setParameter(string identifier, float value) 
{
}

LossyDetector::ProgramList
LossyDetector::getPrograms() const
{
    ProgramList list;
    return list;
}

string
LossyDetector::getCurrentProgram() const
{
    return ""; // no programs
}

void
LossyDetector::selectProgram(string name)
{
}

LossyDetector::OutputList
LossyDetector::getOutputDescriptors() const
{
    OutputList list;

    int outputNo = 0;

    OutputDescriptor d;
    d.identifier = "lossy";
    d.name = "Lossy";
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
    d.hasDuration = true;
    m_lossyOutput = outputNo++;
    list.push_back(d);

    d.identifier = "cf";
    d.name = "Classification function";
    d.description = "A series of values indicating the changing detected likelihood of lossy encoding, from 0 (believed to be lossless) to 1 (believed lossy).";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = true;
    d.minValue = 0.f;
    d.maxValue = 1.f;
    d.isQuantized = false;
    d.quantizeStep = 0.f;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = m_inputSampleRate / float(m_imageWidth);
    d.hasDuration = false;
    m_functionOutput = outputNo++;
    list.push_back(d);

    return list;
}

bool
LossyDetector::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!

    return true;
}

void
LossyDetector::reset()
{
    m_buildingImage = {};
    m_lossyCount = 0;
    m_totalCount = 0;
    m_lastTimestamp = Vamp::RealTime::zeroTime;
}

LossyDetector::FeatureSet
LossyDetector::process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp)
{
    m_lastTimestamp = timestamp;
    
    // The pipeline used in training is
    // 
    // 1. Open file at native rate
    // 2. Mix to mono
    // 3. Frame with 512 blocksize, 256 hop (regardless of sample rate)
    // 4. Window with a periodic Hann window
    // 5. Forward FFT of size 512. (So far, the Vamp host SDK can be
    //    assumed to have managed this for us)
    // 6. Take magnitudes
    // 7. Scale by sqrt(512) - Note the Vamp SDK doesn't actually say
    //    anything about frequency-domain input scaling. I think in
    //    practice it is generally unscaled?
    // 8. Convert to dB (using 20 * log10)
    // 9. Scale by 1/120, add 1, clamp to range 0-1
    // 10. (or between any two steps from 5 onward) Take a section as a
    //    matrix of width 172 and height 257 - in training we do this
    //    three times, starting at 31, 74, and 118.4 seconds in
    // 11. Run classifier

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

    Vamp::RealTime rtWidth = Vamp::RealTime::frame2RealTime
        ((m_imageWidth - 1) * (m_blockSize/2), m_inputSampleRate);
    
    FeatureSet fs;
    
    if (int(m_buildingImage.size()) < m_imageWidth) {
        return fs;
    }
    
    t_1 result = classify(m_buildingImage);

    m_buildingImage = {};

    Feature f;

    f.hasTimestamp = true;
    f.timestamp = timestamp - rtWidth;
    f.hasDuration = false;
    f.values.push_back(result[0]); // result probs are ordered lossy, original

    if (result[0] < 0.5f) {
        f.label = "Original";
    } else {
        f.label = "Lossy";
        ++m_lossyCount;
    }

    ++m_totalCount;
    
    fs[m_functionOutput].push_back(f);

    return fs;
}

LossyDetector::FeatureSet
LossyDetector::getRemainingFeatures()
{
    Feature f;

    f.hasTimestamp = true;
    f.timestamp = Vamp::RealTime::zeroTime;
    f.hasDuration = true;
    f.duration = m_lastTimestamp;

    if (m_lossyCount * 4 >= m_totalCount) {
        f.values.push_back(1.f);
        f.label = "Lossy";
    } else {
        f.values.push_back(0.f);
        f.label = "Original";
    }
    
    FeatureSet fs;
    fs[m_lossyOutput].push_back(f);
    return fs;
}

