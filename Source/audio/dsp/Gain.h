#pragma once
#include "PRM.h"

namespace dsp
{
	template<float SmoothLengthMs>
	struct Gain
	{
		static constexpr float MinDb = -120.f;

		// defaultValueDb
		Gain(float);

		// sampleRate
		void prepare(float) noexcept;

		// bufferView, gainDb, numChannels, numSamples
		void operator()(BufferView2, float, int, int) noexcept;

		void operator()(ProcessorBufferView&, float) noexcept;

		// view
		void applyInverse(ProcessorBufferView&) noexcept;

		// smpls, numSamples
		void applyInverse(float*, int) noexcept;

	private:
		PRM gainPRM;
		PRMInfo gainInfo;
	};
}