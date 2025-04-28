#pragma once
#include "Smooth.h"

namespace dsp
{
	using Smooth = smooth::Smooth;

	struct PRMInfo
	{
		// buf, val, smoothing
		PRMInfo(float* = nullptr, float = 0.f, bool = false);

		// idx
		float operator[](int) const noexcept;

		// numSamples
		void copyToBuffer(int) noexcept;

		// startIdx, endIdx
		void copyToBuffer(int, int) noexcept;

		operator float() const noexcept;

		float* buf;
		float val;
		bool smoothing;
	};
	
	struct PRM
	{
		// startVal
		PRM(float = 0.f);
		
		// sampleRate, smoothLenMs
		void prepare(float sampleRate, float smoothLenMs) noexcept;

		// value, numSamples
		PRMInfo operator()(float, int) noexcept;

		// value, startIdx, endIdx
		PRMInfo operator()(float, int, int) noexcept;

		// startIdx, endIdx
		PRMInfo operator()(int, int) noexcept;

		// numSamples
		PRMInfo operator()(int) noexcept;

		// idx
		float operator[](int) const noexcept;

		std::array<float, BlockSize> buf;
		Smooth smooth;
		float value;
		bool smoothing;
	};

	struct PRMBlock
	{
		// startVal
		PRMBlock(float = 0.f);

		// sampleRate, smoothLenMs
		void prepare(float, float) noexcept;

		void reset() noexcept;

		// val
		void reset(float) noexcept;

		// value
		PRMInfo operator()(float) noexcept;

		operator float() const noexcept;

		float startVal;
		smooth::LowpassG0 lp;
		PRMInfo info;
	};

	struct PRMBlockStereo
	{
		// startVal
		PRMBlockStereo(float = 0.f);

		// sampleRate, smoothLenMs
		void prepare(float, float) noexcept;

		// value, ch
		PRMInfo operator()(float, int) noexcept;

		// ch
		PRMInfo operator[](int) const noexcept;

	protected:
		std::array<PRMBlock, 2> prms;
	};
}