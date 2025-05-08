#pragma once
#include "Transport.h"
#include "Whead.h"
#include "Phasor.h"

namespace dsp
{
	struct Oscilloscope
	{
		Oscilloscope() :
			wHead(),
			buffer(),
			phasor(0.),
			beatLength(1.f),
			Fs(0.),
			scEnabled(true)
		{}

		void prepare(double sampleRate)
		{
			Fs = sampleRate;
			auto windowSize = static_cast<int>(Fs) * 4;
			wHead.prepare(windowSize);
			buffer.resize(windowSize, 0.f);
			phasor.prepare(1. / Fs);
		}

		void operator()(const ProcessorBufferView& view, const Transport::Info& transport) noexcept
		{
			const auto numChannels = scEnabled ? view.getNumChannelsSC() : view.getNumChannelsMain();
			const auto numSamples = view.getNumSamples();
			
			wHead(numSamples);

			const auto rateSyncV = 1.;

			const auto bpm = transport.bpm;
			const auto bps = bpm * .0166666667;
			const auto quarterNoteLengthInSamples = Fs / bps;
			const auto barLengthInSamples = quarterNoteLengthInSamples * 4.;
			const auto beatLen = barLengthInSamples * rateSyncV;
			phasor.inc = 1. / beatLen;
			beatLength.store(static_cast<float>(beatLen));

			const auto ppq = transport.ppq * .25;
			const auto ppqCh = ppq / rateSyncV;

			phasor.phase.phase = ppqCh - std::floor(ppqCh);

			{
				const auto smpls = scEnabled ? view.getSamplesSC(0) : view.getSamplesMain(0);
				for (auto s = 0; s < numSamples; ++s)
				{
					auto w = wHead[s];

					const auto phaseInfo = phasor();
					if (phaseInfo.retrig)
					{
						wHead.shift(-w, numSamples);
						w = wHead[s];
					}
					buffer[w] = smpls[s];
				}
			}

			if (numChannels == 2)
			{
				const auto smpls = scEnabled ? view.getSamplesSC(1) : view.getSamplesMain(1);
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];
					buffer[w] = (buffer[w] + smpls[s]) * .5f;
				}
			}
		}

		const float* data() const noexcept
		{
			return buffer.data();
		}

		const size_t windowLength() const noexcept
		{
			return buffer.size();
		}

		const float getBeatLength() const noexcept
		{
			return beatLength.load();
		}
	private:
		WHead wHead;
		std::vector<float> buffer;
		Phasor phasor;
		std::atomic<float> beatLength;
		double Fs;
		bool scEnabled;
	};
}