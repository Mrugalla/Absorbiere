#pragma once
#include <random>
#include "Transport.h"
#include "PRM.h"
#include "Phasor.h"
#include "XFade.h"

namespace perlin
{
	static constexpr int NumOctaves = 8;
	static constexpr int NoiseOvershoot = 4;

	static constexpr int NoiseSize = 1 << NumOctaves;
	static constexpr int NoiseSizeMax = NoiseSize - 1;
	static constexpr int BlockSize = dsp::BlockSize;
	using PRMInfo = dsp::PRMInfo;
	using Phasor = dsp::Phasor;
	using SIMD = dsp::SIMD;
	using BufferView2 = dsp::BufferView2;
	using NoiseArray = std::array<float, NoiseSize + NoiseOvershoot>;
	using GainBuffer = std::array<float, NumOctaves + 2>;

	struct Perlin
	{
		Perlin();

		// newPhase
		void updatePosition(double) noexcept;

		// rateHzInv
		void updateSpeed(double) noexcept;

		//smpls, noise, gainBuffer,
		//octavesInfo, phsInfo, widthInfo,
		//shape, numChannels, numSamples
		void operator()(float*, const float*, const float*,
			const PRMInfo&, float, int) noexcept;

		Phasor phasor;
		std::array<float, BlockSize> phaseBuffer;
		int noiseIdx;
	private:
		// phsInfo, numSamples
		void synthesizePhasor(const PRMInfo&, int) noexcept;

		// numSamples
		void synthesizePhasor(int) noexcept;

		float getInterpolatedSample(const float*,
			float, float) const noexcept;

		// smpls, octavesInfo, noise, gainBuffer, shape, numSamples
		void processOctaves(float*, const PRMInfo&,
			const float*, const float*, float, int) noexcept;

		// smpls, noise, gainBuffer, octaves, shape, numSamples
		void processOctavesNotSmoothing(float*, const float*,
			const float*, float,
			float, int) noexcept;

		// smpls, octavesBuf, noise, gainBuffer, shape, numSamples
		void processOctavesSmoothing(float*, const float*,
			const float*, const float*,
			float, int) noexcept;

		// samples, octavesInfo, widthInfo, noise, gainBuffer, shape, numSamples
		void processWidth(BufferView2, const PRMInfo&,
			const PRMInfo&, const float*, const float*,
			float, int) noexcept;

		// phaseInfo, o
		float getPhaseOctaved(float, int) const noexcept;

		// debug:
#if JUCE_DEBUG
		// smpls, numSamples, threshold
		void discontinuityJassert(float*, int, float = .1f);

		// samples, numChannels, numSamples
		void controlRange(float* const*, int, int) noexcept;
#endif
	};

	static constexpr float XFadeLengthMs = 200.f;
	static constexpr int NumPerlins = 3;

	using PRM = dsp::PRM;
	using Mixer = dsp::XFadeMixer<NumPerlins, true>;
	using Perlins = std::array<Perlin, NumPerlins>;
	using Int64 = juce::int64;
	static constexpr float Pi = dsp::Pi;
	using Transport = dsp::Transport::Info;

	struct Perlin2
	{
		Perlin2();

		void setSeed(int);

		// sampleRate
		void prepare(double);

		// smpls, numSamples, transport,
		// rateBeats, octaves, bias[0,1], shape
		void operator()(float*, int,
			const Transport&,
			float, float, float,
			float) noexcept;

		Mixer mixer;
		// misc
		double sampleRateInv;
		// noise
		NoiseArray noise;
		GainBuffer gainBuffer, gainBufferOct;
		Perlins perlins;
		// parameters
		PRM octavesPRM, widthPRM, phsPRM;
		double rateBeats, rateHz, inc, bpm, bps, rateInv;
		// seed
		std::atomic<int> seed;
		// project position
		Int64 posEstimate;

		// transport, rateBeats, rateHz, timeInSamples, temposync
		void updatePerlin(const Transport&,
			double, double, int, bool) noexcept;

		// transport, rateBeats, numSamples
		void updatePerlin(const Transport&,
			float, int) noexcept;

		// bpm, rateHz, rateBeats, timeInSamples, temposync
		void updateSpeed(double, double, double, Int64, bool) noexcept;

		// bpm, rateBeats, timeInSamples
		void updateSpeed(double, double, Int64) noexcept;

		// perlin, ppq, timeSecs, temposync
		void updatePosition(Perlin&, double, double, bool) noexcept;

		// perlin, ppq
		void updatePositionSync(Perlin&, double) noexcept;

		// CROSSFADE FUNCS
		// timeSamples
		bool isLooping(Int64) noexcept;

		// bpm, inc
		const bool keepsSpeed(double, double) const noexcept;

		// bpm, inc
		const bool changesSpeed(double, double) const noexcept;

		// inc, bpm, bps, rateInv, rateHz, rateBeats
		void initXFade(double, double, double,
			double, double, double) noexcept;

		// samples, bias, numChannels, numSamples
		void processBias(float* const*, float,
			int, int) noexcept;

		// smpls, bias, numSamples
		void processBias(float*, float,
			int) noexcept;

		// samples, octaves, numChannels, numSamples
		void fuckingApplyGainMate(float**, float,
			int, int) noexcept;

		// smpls, octaves, numSamples
		void fuckingApplyGainMate(float*, float, int) noexcept;
	};
}