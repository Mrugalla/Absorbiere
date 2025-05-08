#pragma once
#include "audio/PluginProcessor.h"
#include "audio/dsp/MixProcessor.h"
#if PPDHasSidechain
#include "audio/dsp/SCGain.h"
#endif

namespace audio
{
    using ChannelSet = juce::AudioChannelSet;
    using MidiBuffer = juce::MidiBuffer;
	using AudioBufferF = juce::AudioBuffer<float>;
    using AudioBufferD = juce::AudioBuffer<double>;
    using Timer = juce::Timer;
    using SIMD = juce::FloatVectorOperations;
	using String = juce::String;
	using MemoryBlock = juce::MemoryBlock;
    
	using XenManager = arch::XenManager;
    using State = arch::State;
    using Param = param::Param;
    using Params = param::Params;
    using PID = param::PID;
    
    struct Processor :
        public juce::AudioProcessor,
        public Timer
    {
        using BusesProps = juce::AudioProcessor::BusesProperties;
        using ValueTree = juce::ValueTree;

        BusesProps makeBusesProps();
        bool canAddBus(bool) const override;
		
        Processor();
        ~Processor() override;
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        bool isBusesLayoutSupported(const BusesLayout&) const override;
        
        void processBlock(AudioBufferF&, MidiBuffer&) override;
        void processBlockBypassed(AudioBufferF&, MidiBuffer&) override;
        
        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override;
        const String getName() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;
        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int) override;
        const String getProgramName(int) override;
        void changeProgramName(int, const String&) override;
        void getStateInformation(MemoryBlock&) override;
        void setStateInformation(const void*, int) override;
        void timerCallback() override;
        bool supportsDoublePrecisionProcessing() const override;
        void forcePrepare();
#if PPDHasTuningEditor
        XenManager xenManager;
#endif
        Params params;
        State state;
        dsp::Transport transport;
        dsp::PluginProcessor pluginProcessor;
        MidiBuffer midiSubBuffer;
        dsp::MixProcessor mixProcessor;
#if PPDHasSidechain
        dsp::SCGain scGainer;
#endif

        //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
    };
}
