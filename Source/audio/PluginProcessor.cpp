#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		scope(),
		ducker()
	{
		auto& duckRingModBlendParam = params(PID::DuckRingModBlend);
		duckRingModBlendParam.callback = [&](CB cb)
		{
			ducker.setRMBlend(cb.norm);
		};

		auto& duckCompBlendParam = params(PID::DuckCompBlend);
		duckCompBlendParam.callback = [&](CB cb)
		{
			ducker.setCompBlend(cb.norm);
		};

		auto& duckCompThresholdParam = params(PID::DuckCompThreshold);
		duckCompThresholdParam.callback = [&](CB cb)
		{
			ducker.setCompThreshold(cb.denorm());
		};

		auto& duckCompRatioParam = params(PID::DuckCompRatio);
		duckCompRatioParam.callback = [&](CB cb)
		{
			ducker.setCompRatio(cb.denorm());
		};

		auto& duckCompKneeParam = params(PID::DuckCompKnee);
		duckCompKneeParam.callback = [&](CB cb)
		{
			ducker.setCompKnee(cb.denorm());
		};

		auto& duckCompAtkParam = params(PID::DuckCompAttack);
		duckCompAtkParam.callback = [&](CB cb)
		{
			ducker.setCompAtk(cb.denorm(), cb.numChannels);
		};

		auto& duckCompRlsParam = params(PID::DuckCompRelease);
		duckCompRlsParam.callback = [&](CB cb)
		{
			ducker.setCompRls(cb.denorm(), cb.numChannels);
		};

		auto& duckCompBPBlend = params(PID::DuckCompBPBlend);
		duckCompBPBlend.callback = [&](CB cb)
		{
			ducker.setCompBPBlend(cb.norm);
		};

		auto& duckCompBPLowParam = params(PID::DuckCompBPFreqLow);
		duckCompBPLowParam.callback = [&](CB cb)
		{
			const auto pitch = cb.denorm();
			ducker.setCompBPFreqLow(math::noteToFreqHz2(pitch), cb.numChannels);
		};

		auto& duckCompBPHighParam = params(PID::DuckCompBPFreqHigh);
		duckCompBPHighParam.callback = [&](CB cb)
		{
			const auto pitch = cb.denorm();
			ducker.setCompBPFreqHigh(math::noteToFreqHz2(pitch), cb.numChannels);
		};

		auto& duckCompBPListenParam = params(PID::DuckCompBPListen);
		duckCompBPListenParam.callback = [&](CB cb)
		{
			ducker.setCompBPListen(cb.getBool());
		};
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		scope.prepare(sampleRate);
		ducker.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		MidiBuffer&, const Transport::Info& trans) noexcept
	{
		scope(view, trans);
		ducker(view);
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