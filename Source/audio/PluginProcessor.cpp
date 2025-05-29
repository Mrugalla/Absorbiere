#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		scope(),
		ducker()
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		scope.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		MidiBuffer&, const Transport::Info& trans) noexcept
	{
		ducker(view);
		scope(view, trans);
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