#include "Perlin.h"

namespace perlin
{
	void generateProceduralNoise(float* noise, int size, unsigned int seed)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(0.f, 1.f);

		for (auto s = 0; s < size; ++s, ++seed)
		{
			mt.seed(seed);
			noise[s] = dist(mt);
		}
	}

	float applyBias(float x, float bias) noexcept
	{
		if (bias == 0.)
			return x;
		const auto X = 2.f * x * x * x * x * x;
		const auto Y = X * X * X;
		return x + bias * (math::tanhApprox(Y) - x);
	}

	void applyBias(float* smpls, float bias, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = applyBias(smpls[s], bias);
	}

	void applyBias(float* const* samples, float bias,
		int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			applyBias(samples[ch], bias, numSamples);
	}

	void applyBias(float* smpls, const float* bias, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = applyBias(smpls[s], bias[s]);
	}

	void applyBias(float* const* samples, const float* bias, int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			applyBias(samples[ch], bias, numSamples);
	}

	float getInterpolatedNN(const float* noise, float phase) noexcept
	{
		return noise[static_cast<int>(std::round(phase)) + 1];
	}

	float getInterpolatedLerp(const float* noise, float phase) noexcept
	{
		return math::lerp(noise, phase + 1.5f);
	}

	float getInterpolatedSpline(const float* noise, float phase) noexcept
	{
		return math::cubicHermiteSpline(noise, phase);
	}

	// PERLIN

	Perlin::Perlin() :
		// phase
		phasor(),
		phaseBuffer(),
		noiseIdx(0)
	{
	}

	void Perlin::updatePosition(double newPhase) noexcept
	{
		const auto newPhaseFloor = std::floor(newPhase);
		noiseIdx = static_cast<int>(newPhaseFloor) & NoiseSizeMax;
		phasor.phase.phase = newPhase - newPhaseFloor;
	}

	void Perlin::updateSpeed(double rateHzInv) noexcept
	{
		phasor.inc = rateHzInv;
	}

	void Perlin::operator()(float* smpls, const float* noise, const float* gainBuffer,
		const PRMInfo& octavesInfo, float shape, int numSamples) noexcept
	{
		synthesizePhasor(numSamples);
		processOctaves(smpls, octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::synthesizePhasor(const PRMInfo& phsInfo, int numSamples) noexcept
	{
		const auto noiseIdxF = static_cast<float>(noiseIdx);
		if (!phsInfo.smoothing)
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phaseInfo = phasor();
				if (phaseInfo.retrig)
					noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

				const auto phaseF = static_cast<float>(phaseInfo.phase);
				phaseBuffer[s] = phaseF + phsInfo.val + noiseIdxF;
			}
		else
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phaseInfo = phasor();
				if (phaseInfo.retrig)
					noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

				const auto phaseF = static_cast<float>(phaseInfo.phase);
				phaseBuffer[s] = phaseF + phsInfo[s] + noiseIdxF;
			}
	}

	void Perlin::synthesizePhasor(int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto phaseInfo = phasor();
			if (phaseInfo.retrig)
				noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

			const auto phaseF = static_cast<float>(phaseInfo.phase);
			phaseBuffer[s] = phaseF + static_cast<float>(noiseIdx);
		}
	}

	void Perlin::processOctaves(float* smpls, const PRMInfo& octavesInfo,
		const float* noise, const float* gainBuffer, float shape, int numSamples) noexcept
	{
		if (!octavesInfo.smoothing)
			processOctavesNotSmoothing(smpls, noise, gainBuffer, octavesInfo.val, shape, numSamples);
		else
			processOctavesSmoothing(smpls, octavesInfo.buf, noise, gainBuffer, shape, numSamples);
	}

	float Perlin::getInterpolatedSample(const float* noise,
		float phase, float shape) const noexcept
	{
		const auto smplNN = getInterpolatedNN(noise, phase);
		const auto smplCubic = getInterpolatedSpline(noise, phase);
		return smplNN + shape * (smplCubic - smplNN);
	}

	void Perlin::processOctavesNotSmoothing(float* smpls, const float* noise,
		const float* gainBuffer, float octaves, float shape, int numSamples) noexcept
	{
		const auto octFloor = std::floor(octaves);

		for (auto s = 0; s < numSamples; ++s)
		{
			auto sample = 0.f;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;
		}

		auto gain = 0.f;
		for (auto o = 0; o < octFloor; ++o)
			gain += gainBuffer[o];

		const auto octFrac = octaves - octFloor;
		if (octFrac != 0.f)
		{
			const auto octFloorInt = static_cast<int>(octFloor);

			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];;
			}

			gain += octFrac * gainBuffer[octFloorInt];
		}

		SIMD::multiply(smpls, 1.f / std::sqrt(gain), numSamples);
	}

	void Perlin::processOctavesSmoothing(float* smpls, const float* octavesBuf,
		const float* noise, const float* gainBuffer,
		float shape, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto octFloor = std::floor(octavesBuf[s]);

			auto sample = 0.f;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;

			auto gain = 0.f;
			for (auto o = 0; o < octFloor; ++o)
				gain += gainBuffer[o];

			const auto octFrac = octavesBuf[s] - octFloor;
			if (octFrac != 0.f)
			{
				const auto octFloorInt = static_cast<int>(octFloor);

				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];

				gain += octFrac * gainBuffer[octFloorInt];
			}

			smpls[s] /= std::sqrt(gain);
		}
	}

	void Perlin::processWidth(BufferView2 samples, const PRMInfo& octavesInfo,
		const PRMInfo& widthInfo, const float* noise, const float* gainBuffer,
		float shape, int numSamples) noexcept
	{
		if (!widthInfo.smoothing)
			if (widthInfo.val == 0.f)
				return SIMD::copy(samples[1], samples[0], numSamples);
			else
				SIMD::add(phaseBuffer.data(), widthInfo.val, numSamples);
		else
			SIMD::add(phaseBuffer.data(), widthInfo.buf, numSamples);

		processOctaves(samples[1], octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	float Perlin::getPhaseOctaved(float phaseInfo, int o) const noexcept
	{
		const auto ox2 = 1 << o;
		const auto oPhase = phaseInfo * static_cast<float>(ox2);
		const auto oPhaseFloor = std::floor(oPhase);
		const auto oPhaseInt = static_cast<int>(oPhaseFloor) & NoiseSizeMax;
		return oPhase - oPhaseFloor + static_cast<float>(oPhaseInt);
	}

	// debug:
#if JUCE_DEBUG
	void discontinuityJassert(double* smpls, int numSamples, double threshold)
	{
		auto lastSample = smpls[0];
		for (auto s = 1; s < numSamples; ++s)
		{
			auto curSample = smpls[s];
			oopsie(std::abs(curSample - lastSample) > threshold);
			lastSample = curSample;
		}
	}

	void controlRange(double* const* samples, int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			for (auto s = 0; s < numSamples; ++s)
			{
				oopsie(std::abs(samples[ch][s]) >= 1.);
				oopsie(std::abs(samples[ch][s]) <= -1.);
			}
	}
#endif

	// PERLIN2

	Perlin2::Perlin2() :
		mixer(),
		// misc
		sampleRateInv(1.),
		// perlin / noise
		noise(),
		gainBuffer(),
		gainBufferOct(),
		perlins(),
		// parameters
		octavesPRM(1.),
		widthPRM(0.),
		phsPRM(0.),
		rateBeats(1.),
		rateHz(1.),
		inc(1.),
		bpm(1.), bps(1.),
		rateInv(1.),
		// noise seed
		seed(),
		// project position
		posEstimate(-1)
	{
		setSeed(420);
		for (auto s = 0; s < NoiseOvershoot; ++s)
			noise[NoiseSize + s] = noise[s];
		for (auto o = 0; o < gainBuffer.size(); ++o)
			gainBuffer[o] = 1.f / static_cast<float>(1 << o);
		for (auto o = 0; o < gainBufferOct.size(); ++o)
		{
			auto g = 0.f;
			for (auto i = 0; i < o; ++i)
				g += gainBuffer[i];
			gainBufferOct[o] = std::sqrt(1.f / g);
		}
	}

	void Perlin2::setSeed(int _seed)
	{
		seed.store(_seed);
		generateProceduralNoise(noise.data(), NoiseSize, static_cast<unsigned int>(_seed));
	}

	void Perlin2::prepare(double fs)
	{
		sampleRateInv = 1. / fs;
		const auto fsF = static_cast<float>(fs);
		mixer.prepare(fsF, XFadeLengthMs);
		octavesPRM.prepare(fsF, 10.f);
		widthPRM.prepare(fsF, 20.f);
		phsPRM.prepare(fsF, 20.f);
	}

	void Perlin2::operator()(float* smpls, int numSamples,
		const Transport& transport,
		float _rateBeats, float octaves, float bias,
		float shape) noexcept
	{
		const auto octavesInfo = octavesPRM(octaves, numSamples);

		updatePerlin(transport, _rateBeats, numSamples);

		{
			auto& track = mixer[0];

			if (track.isEnabled())
			{
				auto xBuffer = mixer(0, numSamples);
				auto xSamples = xBuffer[0];

				perlins[0]
				(
					xSamples,
					noise.data(),
					gainBuffer.data(),
					octavesInfo,
					shape,
					numSamples
				);

				mixer.copy(smpls, 0, numSamples);
			}
			else
				SIMD::clear(smpls, numSamples);

		}

		for (auto i = 1; i < NumPerlins; ++i)
		{
			auto& track = mixer[i];

			if (track.isEnabled())
			{
				auto xBuffer = mixer(i, numSamples);
				auto xSamples = xBuffer[0];

				perlins[i]
				(
					xSamples,
					noise.data(),
					gainBuffer.data(),
					octavesInfo,
					shape,
					numSamples
				);

				mixer.add(smpls, i, numSamples);
			}
		}

		if (bias != 0.f)
			processBias(smpls, bias, numSamples);

		fuckingApplyGainMate(smpls, octaves, numSamples);
	}

	void Perlin2::updatePerlin(const Transport& transport,
		double _rateBeats, double _rateHz, int numSamples, bool temposync) noexcept
	{
		updateSpeed(transport.bpm, _rateHz, _rateBeats, transport.timeSamples, temposync);

		if (transport.playing)
		{
			updatePosition(perlins[mixer.idx], transport.ppq, transport.timeSecs, temposync);
			posEstimate = transport.timeSamples + numSamples;
		}
		else
			posEstimate = transport.timeSamples;
	}

	void Perlin2::updatePerlin(const Transport& transport,
		float _rateBeats, int numSamples) noexcept
	{
		updateSpeed(transport.bpm, static_cast<double>(_rateBeats), transport.timeSamples);

		if (transport.playing)
		{
			updatePositionSync(perlins[mixer.idx], transport.ppq);
			posEstimate = transport.timeSamples + numSamples;
		}
		else
			posEstimate = transport.timeSamples;
	}

	void Perlin2::updateSpeed(double nBpm, double nRateHz, double nRateBeats,
		Int64 timeInSamples, bool temposync) noexcept
	{
		double nBps = nBpm / 60.;
		const auto nRateInv = .25 * nRateBeats;

		double nInc = 0.;
		if (temposync)
		{
			const auto bpSamples = nBps * sampleRateInv;
			nInc = nRateInv * bpSamples;
		}
		else
			nInc = nRateHz * sampleRateInv;

		if (isLooping(timeInSamples) || (changesSpeed(nBpm, nInc) && !mixer.stillFading()))
			initXFade(nInc, nBpm, nBps, nRateInv, nRateHz, nRateBeats);
	}

	void Perlin2::updateSpeed(double nBpm, double nRateBeats, Int64 timeInSamples) noexcept
	{
		double nBps = nBpm / 60.;
		const auto nRateInv = .25 * nRateBeats;

		const auto bpSamples = nBps * sampleRateInv;
		const auto nInc = nRateInv * bpSamples;

		if (isLooping(timeInSamples) || (changesSpeed(nBpm, nInc) && !mixer.stillFading()))
			initXFade(nInc, nBpm, nBps, nRateInv, 0., nRateBeats);
	}

	void Perlin2::updatePosition(Perlin& perlin, double ppqPosition,
		double timeInSecs, bool temposync) noexcept
	{
		if (temposync)
			updatePositionSync(perlin, ppqPosition);
		else
		{
			const auto nPhase = timeInSecs * rateHz;
			perlin.updatePosition(nPhase);
		}
	}

	void Perlin2::updatePositionSync(Perlin& perlin, double ppqPosition) noexcept
	{
		const auto ppq = ppqPosition - (bps * sampleRateInv);
		const auto nPhase = ppq * rateInv + .5;
		perlin.updatePosition(nPhase);
	}

	// CROSSFADE FUNCS
	bool Perlin2::isLooping(Int64 timeInSamples) noexcept
	{
		const auto error = std::abs(timeInSamples - posEstimate);
		return error > 1;
	}

	const bool Perlin2::keepsSpeed(double nBpm, double nInc) const noexcept
	{
		return nInc == inc && bpm == nBpm;
	}

	const bool Perlin2::changesSpeed(double nBpm, double nInc) const noexcept
	{
		return !keepsSpeed(nBpm, nInc);
	}

	void Perlin2::initXFade(double nInc, double nBpm, double nBps,
		double nRateInv, double _rateHz, double _rateBeats) noexcept
	{
		inc = nInc;
		bpm = nBpm;
		bps = nBps;
		rateInv = nRateInv;
		rateHz = _rateHz;
		rateBeats = _rateBeats;
		mixer.init();
		perlins[mixer.idx].updateSpeed(inc);
	}

	void Perlin2::processBias(float* const* samples, float bias,
		int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			processBias(samples[ch], bias, numSamples);
	}

	void Perlin2::processBias(float* smpls, float bias,
		int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto x = smpls[s];
			const auto a = std::abs(2.f * (x - .5f));
			const auto b = (1.f + bias * 12.f) * Pi;
			const auto c = b * a;
			if (c == 0.f)
				smpls[s] = 1.f;
			else
			{
				const auto d = std::sin(c) / c;
				const auto e = d * d;
				smpls[s] = x + bias * (e - x);
			}
		}
	}

	void Perlin2::fuckingApplyGainMate(float** samples, float octaves,
		int numChannels, int numSamples) noexcept
	{
		const auto oFloor = std::floor(octaves);
		const auto o0 = static_cast<int>(oFloor);
		const auto o1 = static_cast<int>(oFloor + 1);
		const auto oFrac = octaves - oFloor;
		const auto og0 = gainBufferOct[o0];
		const auto og1 = gainBufferOct[o1];
		const auto ogRange = og1 - og0;
		const auto oGain = og0 + oFrac * ogRange;
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = samples[ch];
			SIMD::multiply(smpls, oGain, numSamples);
			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = math::limit(0.f, 1.f, smpls[s]);
		}
	}

	void Perlin2::fuckingApplyGainMate(float* smpls, float octaves, int numSamples) noexcept
	{
		const auto oFloor = std::floor(octaves);
		const auto o0 = static_cast<int>(oFloor);
		const auto o1 = static_cast<int>(oFloor + 1);
		const auto oFrac = octaves - oFloor;
		const auto og0 = gainBufferOct[o0];
		const auto og1 = gainBufferOct[o1];
		const auto ogRange = og1 - og0;
		const auto oGain = og0 + oFrac * ogRange;
		SIMD::multiply(smpls, oGain, numSamples);
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = math::limit(0.f, 1.f, smpls[s]);
	}
}