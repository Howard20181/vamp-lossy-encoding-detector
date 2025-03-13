
#include "LossyDetector.h"

#include "version.h"


LossyDetector::LossyDetector(float inputSampleRate) :
    Plugin(inputSampleRate)
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
    return 512;
}

size_t 
LossyDetector::getPreferredStepSize() const
{
    return 256;
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
    d.hasDuration = false;
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
    // Clear buffers, reset stored values, etc
}

LossyDetector::FeatureSet
LossyDetector::process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp)
{
    // The pipeline used in training is
    // 
    // 1. Open file at native rate
    // 2. Take first channel. (Later perhaps: mix to mono. Let's mix
    //    to mono here because that's what we get anyway with channels
    //    set to 1, then compare)
    // 3. Frame with 512 blocksize, 256 hop (regardless of sample rate)
    // 4. Window with a periodic Hann window
    // 5. Forward FFT of size 512. (So far, the Vamp host SDK can be
    //    assumed to have managed this for us)
    // 6. Scale by sqrt(512) - Note the Vamp SDK doesn't actually say
    //    anything about frequency-domain input scaling. I think in
    //    practice it is generally unscaled?
    // 7. Convert to dB (using 20 * log10)
    // 8. Scale by 1/120, add 1, clamp to range 0-1
    // 9. (or between any two steps from 5 onward) Take a section as a
    //    matrix of width 172 and height 257 - in training we do this
    //    three times, starting at 31, 74, and 118.4 seconds in
    // 10. Run classifier

    
    
    return FeatureSet();
}

LossyDetector::FeatureSet
LossyDetector::getRemainingFeatures()
{
    return FeatureSet();
}

