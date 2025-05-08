#pragma once

namespace dsp
{
	static constexpr auto ThresholdMin = -60, ThresholdMax = 0;
	static constexpr auto RatioMin = 1, RatioMax = 60;
	static constexpr auto KneeMin = 0, KneeMax = 20;

	// https://www.desmos.com/calculator/iygmeo0ozx
	static inline float TransferDownwardsComp(float x,
		float threshold, float ratio, float knee) noexcept
	{
		const auto k = knee * .5f;
		const auto t0 = threshold - k;
		if (x < t0)
			return x;
		const auto t1 = threshold + k;
		const auto x0 = x - t0;
		const auto x00 = x0 * x0;
		const auto k2 = knee * 2.f;
		const auto m = (1.f / ratio - 1.f) / k2;
		const auto yk = x + m * x00;
		if (x < t1)
			return yk;
		const auto yR = (x - threshold) / ratio + threshold;
		return yR;
	}
}