
#ifndef QUICK_LOSSY_DETECTOR_H
#define QUICK_LOSSY_DETECTOR_H

#include <vamp-sdk/Plugin.h>

#include "types.h"

#include <deque>

class QuickLossyDetector : public Vamp::Plugin
{
public:
    QuickLossyDetector(float inputSampleRate);
    virtual ~QuickLossyDetector();

    std::string getIdentifier() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getMaker() const;
    int getPluginVersion() const;
    std::string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(std::string identifier) const;
    void setParameter(std::string identifier, float value);

    ProgramList getPrograms() const;
    std::string getCurrentProgram() const;
    void selectProgram(std::string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    mutable int m_lossyOutput;
    int m_blockSize;
    int m_imageWidth;
    std::deque<t_1> m_buildingImage;
    bool m_enough;
    Vamp::RealTime m_lastTimestamp;
};

#endif
