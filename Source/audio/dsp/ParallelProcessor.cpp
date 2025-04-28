#include "ParallelProcessor.h"

namespace dsp
{
	template<size_t NumBands>
	ParallelProcessor<NumBands>::ParallelProcessor() :
		bands(),
		sleepy()
	{
		for (auto& s : sleepy)
			s = true;
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::split(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto b = 0; b < MaxBand; ++b)
		{
			const auto b2 = 2 * b;
			float* band[] = { bands[b2].data(), bands[b2 + 1].data() };

			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::copy(band[ch], view.getSamplesMain(ch), numSamples);
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::join(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto b = 0; b < MaxBand; ++b)
		{
			const auto b2 = 2 * b;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::add(view.getSamplesMain(ch), bands[b2 + ch].data(), numSamples);
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::joinReplace(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto ch = 0; ch < numChannels; ++ch)
			SIMD::copy(view.getSamplesMain(ch), bands[ch].data(), numSamples);

		for (auto b = 1; b < MaxBand; ++b)
		{
			const auto b2 = 2 * b;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::add(view.getSamplesMain(ch), bands[b2 + ch].data(), numSamples);
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::applyGain(float gain, int bandIdx, int numChannels, int numSamples) noexcept
	{
		const auto b2 = 2 * bandIdx;
		float* band[] = { bands[b2].data(), bands[b2 + 1].data() };

		for (auto ch = 0; ch < numChannels; ++ch)
			SIMD::multiply(band[ch], gain, numSamples);
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::applyGain(float* gain, int bandIdx, int numChannels, int numSamples) noexcept
	{
		const auto b2 = 2 * bandIdx;
		for (auto ch = 0; ch < numChannels; ++ch)
			SIMD::multiply(bands[b2 + ch].data(), gain, numSamples);
	}

	template<size_t NumBands>
	BufferView2 ParallelProcessor<NumBands>::getBand(int bandIdx) noexcept
	{
		const auto b2 = 2 * bandIdx;
		return { bands[b2].data(), bands[b2 + 1].data() };
	}

	template<size_t NumBands>
	BufferView2 ParallelProcessor<NumBands>::operator[](int bandIdx) noexcept
	{
		return getBand(bandIdx);
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::joinMix(ProcessorBufferView& view, float* mix) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = view.getSamplesMain(ch);
			auto band = bands[ch].data();

			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = band[s] + mix[s] * (smpls[s] - band[s]);
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::joinMix(ProcessorBufferView& view, float mix) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = view.getSamplesMain(ch);
			auto band = bands[ch].data();

			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = band[s] + mix * (smpls[s] - band[s]);
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::joinDelta(ProcessorBufferView& view, float* gain) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = view.getSamplesMain(ch);
			auto band = bands[ch].data();

			for (auto s = 0; s < numSamples; ++s)
			{
				const auto g = gain[s];
				const auto dry = band[s];
				const auto wet = smpls[s];
				const auto delta = wet - dry;

				smpls[s] = g * delta;
			}
		}
	}

	template<size_t NumBands>
	void ParallelProcessor<NumBands>::joinDelta(ProcessorBufferView& view, float gain) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = view.getSamplesMain(ch);
			auto band = bands[ch].data();

			for (auto s = 0; s < numSamples; ++s)
			{
				const auto dry = band[s];
				const auto wet = smpls[s];
				const auto delta = wet - dry;

				smpls[s] = gain * delta;
			}
		}
	}

	template<size_t NumBands>
	bool ParallelProcessor<NumBands>::isSleepy(int idx) const noexcept
	{
		return sleepy[idx];
	}
	
	template<size_t NumBands>
	void ParallelProcessor<NumBands>::setSleepy(bool e, int idx) noexcept
	{
		sleepy[idx] = e;
	}

	template struct ParallelProcessor<2>;
	template struct ParallelProcessor<3>;
	template struct ParallelProcessor<4>;
	template struct ParallelProcessor<5>;
	template struct ParallelProcessor<NumMPEChannels>;
	template struct ParallelProcessor<NumMIDIChannels>;
}
