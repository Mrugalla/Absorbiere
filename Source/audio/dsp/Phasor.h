#pragma once

namespace dsp
{
	struct PhaseInfo
	{
		// phase, retrig
		PhaseInfo(double = 0., bool = false);

		double phase;
		bool retrig;
	};

	struct Phasor
	{
		void setFrequencyHz(double) noexcept;

		// phase, inc
		Phasor(double = 0., double = 0.);

		// fsInv
		void prepare(double) noexcept;

		void reset(double = 0.) noexcept;

		PhaseInfo operator()() noexcept;

		// numSamples
		PhaseInfo operator()(double) noexcept;

		PhaseInfo phase;
		double inc, fsInv;
	};
}