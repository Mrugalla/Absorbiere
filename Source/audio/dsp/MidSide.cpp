#include "MidSide.h"

namespace dsp
{
	template<typename Float>
	void midSideEncode(Float* const* samples, int numSamples) noexcept
	{
		static constexpr auto A = static_cast<Float>(.5);
		auto smplsL = samples[0];
		auto smplsR = samples[1];

		for (auto s = 0; s < numSamples; ++s)
		{
			const auto mid = (smplsL[s] + smplsR[s]) * A;
			const auto side = (smplsL[s] - smplsR[s]) * A;
			smplsL[s] = mid;
			smplsR[s] = side;
		}
	}

	template<typename Float>
	void midSideDecode(Float* const* samples, int numSamples) noexcept
	{
		auto smplsM = samples[0];
		auto smplsS = samples[1];

		for (auto s = 0; s < numSamples; ++s)
		{
			const auto left = smplsM[s] + smplsS[s];
			const auto right = smplsM[s] - smplsS[s];
			smplsM[s] = left;
			smplsS[s] = right;
		}
	}

	template void midSideEncode(float* const*, int) noexcept;
	template void midSideEncode(double* const*, int) noexcept;
	template void midSideDecode(float* const*, int) noexcept;
	template void midSideDecode(double* const*, int) noexcept;
}