#pragma once
#include "PRM.h"

namespace dsp
{
	struct EnvelopeFollower
	{
		struct Params
		{
			float gainDb;
			double atkMs, dcyMs, smoothMs;
		};

		EnvelopeFollower();

		void prepare(float) noexcept;

		// smpls, params, numSamples
		void operator()(float*, const Params&, int) noexcept;

		// samples, params, numChannels, numSamples
		void operator()(float**, const Params&, int, int) noexcept;

		bool isSleepy() const noexcept;

		float operator[](int i) const noexcept;

		float getMeter() const noexcept;
	private:
		std::atomic<float> meter;
		std::array<float, BlockSize> buffer;
		const double MinDb;
		PRM gainPRM;
		smooth::LowpassG0 envLP, smooth;
		double sampleRate, smoothMs;
		bool attackState;

		// samples, numChannels, numSamples
		void copyMid(float**, int, int) noexcept;

		// smpls, numSamples
		void rectify(float*, int) noexcept;

		// gainDb, numSamples
		void applyGain(float, int) noexcept;

		// params, numSamples
		void synthesizeEnvelope(const Params&, int) noexcept;

		// params, s0, s1
		double processAttack(const Params&, double, double) noexcept;

		// params, s0, s1
		double processDecay(const Params&, double, double) noexcept;

		// _smoothMs, numSamples
		void smoothen(double, int) noexcept;

		void processMeter() noexcept;
	};
}
