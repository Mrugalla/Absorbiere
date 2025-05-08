#pragma once
#include "dsp/Transport.h"
#include "../param/Param.h"

#include "dsp/Oscilloscope.h"
#include "dsp/Ducker.h"

namespace dsp
{
	using Params = param::Params;
	using PID = param::PID;
	using State = arch::State;
	using CB = param::Param::CB;
#if PPDHasTuningEditor
	using XenManager = arch::XenManager;
#endif
	struct PluginProcessor
	{
		PluginProcessor(Params&
#if PPDHasTuningEditor
			, XenManager&
#endif
		);

		// sampleRate
		void prepare(double);

		// samples, midiBuffer, transport
		void operator()(ProcessorBufferView& buffer, MidiBuffer&, const Transport::Info&) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(float**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		double sampleRate;
		Oscilloscope scope;
		Ducker ducker;
	};
}