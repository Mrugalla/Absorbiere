#pragma once
#include "../Using.h"

namespace dsp
{
	template<size_t NumBands>
	struct ParallelProcessor
	{
		static constexpr int MaxBand = NumBands - 1;
		static constexpr int NumChannels = 2 * MaxBand;

		ParallelProcessor();

		// view
		void split(ProcessorBufferView&) noexcept;

		// view
		void join(ProcessorBufferView&) noexcept;

		// view
		void joinReplace(ProcessorBufferView&) noexcept;

		// gain, bandIdx, numChannels, numSamples
		void applyGain(float, int, int, int) noexcept;

		// gain, bandIdx, numChannels, numSamples
		void applyGain(float*, int, int, int) noexcept;

		BufferView2 getBand(int) noexcept;

		BufferView2 operator[](int) noexcept;

		// only for parallel processors with exactly 2 bands
		// view, mix
		void joinMix(ProcessorBufferView&, float*) noexcept;

		// only for parallel processors with exactly 2 bands
		// view, mix
		void joinMix(ProcessorBufferView&, float) noexcept;

		// only for parallel processors with exactly 2 bands
		// view, gain
		void joinDelta(ProcessorBufferView&, float*) noexcept;

		//only for parallel processors with exactly 2 bands
		// view, gain
		void joinDelta(ProcessorBufferView&, float) noexcept;

		// bandIdx
		bool isSleepy(int) const noexcept;

		// sleepyState, bandIdx
		void setSleepy(bool, int) noexcept;

	private:
		std::array<std::array<float, BlockSize>, NumChannels> bands;
		std::array<bool, NumBands> sleepy;
	};

	using PP2Band = ParallelProcessor<2>;
	using PP3Band = ParallelProcessor<3>;
	using PP4Band = ParallelProcessor<4>;
	using PP5Band = ParallelProcessor<5>;
	using PPMPEBand = ParallelProcessor<NumMPEChannels>;
	using PPMIDIBand = ParallelProcessor<NumMIDIChannels>;
}