#include "Gain.h"
#include "../../arch/Math.h"

namespace dsp
{
	template<float SmoothLengthMs>
	Gain<SmoothLengthMs>::Gain(float defaultValueDb) :
		gainPRM(math::dbToAmp(defaultValueDb, MinDb)),
		gainInfo(nullptr, 0.f, false)
	{}

	template<float SmoothLengthMs>
	void Gain<SmoothLengthMs>::prepare(float sampleRate) noexcept
	{
		gainPRM.prepare(sampleRate, SmoothLengthMs);
	}

	template<float SmoothLengthMs>
	void Gain<SmoothLengthMs>::operator()(ProcessorBufferView& view, float gainDb) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		const auto gainAmp = math::dbToAmp(gainDb, MinDb);
		gainInfo = gainPRM(gainAmp, numSamples);
		if (gainInfo.smoothing)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), gainInfo.buf, numSamples);
			return;
		}
		if (gainInfo.val == 0.f)
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::clear(view.getSamplesMain(ch), numSamples);
		else if (gainInfo.val != 1.f)
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), gainInfo.val, numSamples);
	}

	template<float SmoothLengthMs>
	void Gain<SmoothLengthMs>::operator()(BufferView2 buffer, float gainDb,
		int numChannels, int numSamples) noexcept
	{
		const auto gainAmp = math::dbToAmp(gainDb, MinDb);
		gainInfo = gainPRM(gainAmp, numSamples);
		if (gainInfo.smoothing)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(buffer[ch], gainInfo.buf, numSamples);
			return;
		}
		if (gainInfo.val == 0.f)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::clear(buffer[ch], numSamples);
		}
		else if (gainInfo.val != 1.f)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(buffer[ch], gainInfo.val, numSamples);
		}
	}

	template<float SmoothLengthMs>
	void Gain<SmoothLengthMs>::applyInverse(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		if (gainInfo.smoothing)
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < numSamples; ++s)
					smpls[s] /= gainInfo.buf[s];
			}
		else
		{
			gainInfo.val = 1.f / gainInfo.val;
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				SIMD::multiply(smpls, gainInfo.val, numSamples);
			}
		}
	}

	template<float SmoothLengthMs>
	void Gain<SmoothLengthMs>::applyInverse(float* smpls, int numSamples) noexcept
	{
		if (gainInfo.smoothing)
		{
			for (auto s = 0; s < numSamples; ++s)
				smpls[s] /= gainInfo.buf[s];
			return;
		}
		gainInfo.val = 1.f / gainInfo.val;
		SIMD::multiply(smpls, gainInfo.val, numSamples);
	}

	template struct Gain<1.f>;
	template struct Gain<2.f>;
	template struct Gain<3.f>;
	template struct Gain<5.f>;
	template struct Gain<8.f>;
	template struct Gain<13.f>;
	template struct Gain<21.f>;
	template struct Gain<34.f>;
	template struct Gain<55.f>;
	template struct Gain<89.f>;
	template struct Gain<144.f>;
}