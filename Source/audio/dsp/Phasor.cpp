#include "Phasor.h"

namespace dsp
{
	// PhasorInfo

	PhaseInfo::PhaseInfo(double _phase, bool _retrig) :
		phase(_phase),
		retrig(_retrig)
	{}

	// Phasor

	void Phasor::setFrequencyHz(double hz) noexcept
	{
		inc = hz * fsInv;
	}

	Phasor::Phasor(double _phase, double _inc) :
		phase(_phase, false),
		inc(_inc),
		fsInv(1.)
	{

	}

	void Phasor::prepare(double _fsInv) noexcept
	{
		fsInv = _fsInv;
	}

	void Phasor::reset(double p) noexcept
	{
		phase.phase = p;
	}

	PhaseInfo Phasor::operator()() noexcept
	{
		phase.phase += inc;
		if (phase.phase >= 1.)
		{
			--phase.phase;
			phase.retrig = true;
			return phase;
		}
		phase.retrig = false;
		return phase;
	}

	PhaseInfo Phasor::operator()(double numSamples) noexcept
	{
		phase.phase += inc * numSamples;
		phase.retrig = false;
		while (phase.phase >= 1.)
		{
			--phase.phase;
			phase.retrig = true;
		}
		return phase;
	}
}