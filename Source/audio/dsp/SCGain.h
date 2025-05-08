#pragma once
#include "../Using.h"
#include "../../param/Param.h"

namespace dsp
{
	struct SCGain
	{
		SCGain(param::Param& _scGain) :
			scGain(_scGain),
			listening(false),
			maxPeak(0.f)
		{ }

		void reset() noexcept
		{
			listening.store(false);
		}

		void setListening(bool e) noexcept
		{
			if (listening.load() == e)
				return;
			listening.store(e);
			if (e)
			{
				scGain.beginGesture();
				scGain.setValueNotifyingHost(0.f);
				maxPeak.store(0.f);
			}
			else
			{
				scGain.endGesture();
			}
		}

		void operator()(float* const* samples,
			int numChannels, int numSamples) noexcept
		{
			if (!listening.load())
				return;
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				const auto smpls = samples[ch];
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto y = std::abs(smpls[s]);
					if (maxPeak.load() < y)
					{
						maxPeak.store(y);
						const auto gain = 1.f / y;
						const auto gainDb = math::ampToDecibel(gain);
						const auto gainLegal = scGain.range.snapToLegalValue(gainDb);
						const auto gainNorm = scGain.range.convertTo0to1(gainLegal);
						scGain.setValueNotifyingHost(gainNorm);
					}
				}
			}
		}

		bool isListening() const noexcept
		{
			return listening.load();
		}
	private:
		param::Param& scGain;
		std::atomic<bool> listening;
		std::atomic<float> maxPeak;
	};
}