#include "EnvelopeFollower.h"

namespace dsp
{
	EnvelopeFollower::EnvelopeFollower() :
		meter(0.f),
		buffer(),
		MinDb(math::dbToAmp(-60.)),
		gainPRM(1.f),
		envLP(0.),
		smooth(0.f),
		sampleRate(1.), smoothMs(-1.),
		attackState(false)
	{
	}

	void EnvelopeFollower::prepare(float Fs) noexcept
	{
		meter.store(0.f);
		sampleRate = static_cast<double>(Fs);
		gainPRM.prepare(Fs, 4.);
		smooth.reset();
		smoothMs = -1.;
		envLP.reset();
	}

	void EnvelopeFollower::operator()(float* smpls,
		const Params& params, int numSamples) noexcept
	{
		rectify(smpls, numSamples);
		applyGain(params.gainDb, numSamples);
		synthesizeEnvelope(params, numSamples);
		smoothen(params.smoothMs, numSamples);
		processMeter();
	}

	void EnvelopeFollower::operator()(float** samples, const Params& params,
		int numChannels, int numSamples) noexcept
	{
		copyMid(samples, numChannels, numSamples);
		operator()(buffer.data(), params, numSamples);
	}

	bool EnvelopeFollower::isSleepy() const noexcept
	{
		return !attackState && envLP.y1 < MinDb;
	}

	float EnvelopeFollower::operator[](int i) const noexcept
	{
		return buffer[i];
	}

	float EnvelopeFollower::getMeter() const noexcept
	{
		return meter.load();
	}

	void EnvelopeFollower::copyMid(float** samples, int numChannels, int numSamples) noexcept
	{
		SIMD::copy(buffer.data(), samples[0], numSamples);
		if (numChannels == 1)
			return;
		SIMD::add(buffer.data(), samples[1], numSamples);
		SIMD::multiply(buffer.data(), .5f, numSamples);
	}

	void EnvelopeFollower::rectify(float* smpls, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			buffer[s] = std::abs(smpls[s]);
	}

	void EnvelopeFollower::applyGain(float gainDb, int numSamples) noexcept
	{
		const auto gain = math::dbToAmp(gainDb);
		const auto gainInfo = gainPRM(gain, numSamples);
		auto data = buffer.data();
		if (gainInfo.smoothing)
			return SIMD::multiply(data, gainInfo.buf, numSamples);
		SIMD::multiply(data, gain, numSamples);
	}

	void EnvelopeFollower::synthesizeEnvelope(const Params& params, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto s0 = envLP.y1;
			const auto s1 = static_cast<double>(buffer[s]);
			if (attackState)
				buffer[s] = static_cast<float>(processAttack(params, s0, s1));
			else
				buffer[s] = static_cast<float>(processDecay(params, s0, s1));
		}
	}

	double EnvelopeFollower::processAttack(const Params& params, double s0, double s1) noexcept
	{
		if (s0 <= s1)
			return envLP(s1);
		attackState = false;
		envLP.makeFromDecayInMs(params.dcyMs, sampleRate);
		return processDecay(params, s0, s1);
	}

	double EnvelopeFollower::processDecay(const Params& params, double s0, double s1) noexcept
	{
		if (s0 >= s1)
			return envLP(s1);
		attackState = true;
		envLP.makeFromDecayInMs(params.atkMs, sampleRate);
		return processAttack(params, s0, s1);
	}

	void EnvelopeFollower::smoothen(double _smoothMs, int numSamples) noexcept
	{
		if (smoothMs != _smoothMs)
		{
			smoothMs = _smoothMs;
			smooth.makeFromDecayInMs(smoothMs, sampleRate);
		}
		smooth(buffer.data(), numSamples);
	}

	void EnvelopeFollower::processMeter() noexcept
	{
		const auto max = *std::max_element(buffer.begin(), buffer.end());
		meter.store(max);
	}
}