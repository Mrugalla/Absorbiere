#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(float _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	void PluginProcessor::operator()(ProcessorBufferView& buffer,
		MidiBuffer&, const Transport::Info&) noexcept
	{
		if(buffer.scEnabled)
			for (auto ch = 0; ch < buffer.getNumChannelsMain(); ++ch)
				SIMD::multiply(buffer.getSamplesMain(ch), buffer.getSamplesSC(ch), buffer.numSamples);
	}

	void PluginProcessor::processBlockBypassed(float**, MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch(State&)
	{
	}

	void PluginProcessor::loadPatch(const State&)
	{
	}
}