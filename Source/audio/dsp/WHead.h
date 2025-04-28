#pragma once
#include "../Using.h"

namespace dsp
{
	struct WHead
	{
		WHead();

		// delaySizeSamples
		void prepare(int) noexcept;

		// numSamples
		void operator()(int) noexcept;

		int operator[](int) const noexcept;

		const int* data() const noexcept;

		int* data() noexcept;

		// shift, numSamples
		void shift(int, int) noexcept;
	protected:
		std::array<int, BlockSize> buf;
		int wHead, delaySize;
	};
}