#pragma once
#include "ParallelProcessor.h"
#include "Gain.h"
#include "WHead.h"

namespace dsp
{	
	struct LatencyCompensation
	{
		using DryBuffers = std::array<std::array<float, BlockSize>, 2>;

		LatencyCompensation() :
			ring(),
			wHead(),
			latency(0)
		{}

		void prepare(int _latency)
		{
			latency = _latency;
			if (latency != 0)
			{
				ring.setSize(2, latency, false, true, false);
				wHead.prepare(latency);
			}
			else
			{
				ring.setSize(0, 0);
				wHead.prepare(0);
			}
		}

		void operator()(DryBuffers& dryBuffers, const float* const* inputSamples,
			int numChannels, int numSamples) noexcept
		{
			if (latency != 0)
			{
				wHead(numSamples);

				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto smpls = inputSamples[ch];

					auto rng = ring.getWritePointer(ch);
					auto dry = dryBuffers[ch].data();

					for (auto s = 0; s < numSamples; ++s)
					{
						const auto w = wHead[s];
						const auto r = (w + 1) % latency;

						rng[w] = smpls[s];
						dry[s] = rng[r];
					}
				}
			}
			else
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::copy(dryBuffers[ch].data(), inputSamples[ch], numSamples);
		}

		void operator()(float* const* samples, int numChannels, int numSamples) noexcept
		{
			if (latency != 0)
			{
				wHead(numSamples);

				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto smpls = samples[ch];
					auto rng = ring.getWritePointer(ch);

					for (auto s = 0; s < numSamples; ++s)
					{
						const auto w = wHead[s];
						const auto r = (w + 1) % latency;

						rng[w] = smpls[s];
						smpls[s] = rng[r];
					}
				}
			}
		}

	private:
		AudioBuffer ring;
		WHead wHead;
	public:
		int latency;
	};

	using Gain13 = Gain<13.f>;

	struct MixProcessorNonlinear
	{
		MixProcessorNonlinear() :
			gainIn(0.f)
		{}

		void prepare(float sampleRate)
		{
			gainIn.prepare(sampleRate);
		}

		void split(ProcessorBufferView& view, float gainInDb) noexcept
		{
			gainIn(view, gainInDb);
		}

		void join(ProcessorBufferView& view, bool unityGain) noexcept
		{
			if(unityGain)
				gainIn.applyInverse(view);
		}

		Gain13 gainIn;
	};

	struct MixProcessorDryWet
	{
		MixProcessorDryWet() :
			parallelProcessor(),
			gainDry(0.f),
			gainWetIn(0.f),
			gainWetOut(0.f)
		{}

		void prepare(float sampleRate)
		{
			gainDry.prepare(sampleRate);
			gainWetIn.prepare(sampleRate);
			gainWetOut.prepare(sampleRate);
		}

		// samples, gainDryDb, numChannels, numSamples
		void splitLinear(ProcessorBufferView& view, float gainDryDb) noexcept
		{
			parallelProcessor.split(view);
			const auto band = parallelProcessor.getBand(0);
			gainDry(band, gainDryDb, view.getNumChannelsMain(), view.getNumSamples());
		}

		// samples, gainDryDb, gainWetInDb, numChannels, numSamples
		void splitNonlinear(ProcessorBufferView& view,
			float gainDryDb, float gainWetInDb) noexcept
		{
			splitLinear(view, gainDryDb);
			gainWetIn(view, gainWetInDb);
		}

		// samples, gainWetDb, numChannels, numSamples 
		void joinLinear(ProcessorBufferView& view, float gainWetOutDb) noexcept
		{
			gainWetOut(view, gainWetOutDb);
			parallelProcessor.join(view);
		}

		// samples, gainWetDb, numChannels, numSamples, unityGain
		void joinNonlinear(ProcessorBufferView& view,
			float gainWetOutDb, bool unityGain) noexcept
		{
			if(unityGain)
				gainWetIn.applyInverse(view);
			joinLinear(view, gainWetOutDb);
		}

	private:
		PP2Band parallelProcessor;
		Gain13 gainDry, gainWetIn, gainWetOut;
	};

	struct MixProcessorWetMix
	{
		MixProcessorWetMix() :
			parallelProcessor(),
			gainWetIn(0.f),
			gainWetOut(0.f),
			mixPRM(1.f)
		{}

		void prepare(float sampleRate)
		{
			gainWetIn.prepare(sampleRate);
			gainWetOut.prepare(sampleRate);
			mixPRM.prepare(sampleRate, 13.f);
		}

		// view
		void splitLinear(ProcessorBufferView& view) noexcept
		{
			parallelProcessor.split(view);
		}

		// view, gainWetInDb
		void splitNonlinear(ProcessorBufferView& view, float gainWetInDb) noexcept
		{
			splitLinear(view);
			gainWetIn(view, gainWetInDb);
		}

		// view, mix, gainWetOutDb, delta 
		void joinLinear(ProcessorBufferView& view,
			float mix, float gainWetOutDb, bool delta) noexcept
		{
			gainWetOut(view, gainWetOutDb);
			if (delta)
				joinDelta(view, mix);
			else
				joinMix(view, mix);
		}

		// view, mix, gainWetOutDb, unityGain, delta 
		void joinNonlinear(ProcessorBufferView& view,
			float mix, float gainWetOutDb, bool unityGain, bool delta) noexcept
		{
			if (unityGain)
				gainWetIn.applyInverse(view);
			joinLinear(view, mix, gainWetOutDb, delta);
		};
	
	private:
		PP2Band parallelProcessor;
		Gain13 gainWetIn, gainWetOut;
		PRM mixPRM;

		// view, mix
		void joinMix(ProcessorBufferView& view, float mix) noexcept
		{
			const auto numSamples = view.getNumSamples();
			const auto mixInfo = mixPRM(mix, numSamples);
			if (mixInfo.smoothing)
				parallelProcessor.joinMix(view, mixInfo.buf);
			else
				parallelProcessor.joinMix(view, mixInfo.val);
		}

		// view, mix
		void joinDelta(ProcessorBufferView& view, float mix) noexcept
		{
			const auto numSamples = view.getNumSamples();
			const auto mixInfo = mixPRM(mix, numSamples);
			if (mixInfo.smoothing)
				parallelProcessor.joinDelta(view, mixInfo.buf);
			else
				parallelProcessor.joinDelta(view, mixInfo.val);
		}
	};

	struct MixProcessor
	{
		MixProcessor() :
			mixProcessor(),
			gainOut(0.f)
		{}

		void prepare(float sampleRate)
		{
			mixProcessor.prepare(sampleRate);
			gainOut.prepare(sampleRate);
		}

#if PPDIO == PPDIOOut
	#if PPDIsNonlinear
		// samples, gainInDb, numChannels, numSamples
		void split(float* const* samples, float gainInDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.split(samples, gainInDb, numChannels, numSamples);
		}
		// samples, gainOutDb, numChannels, numSamples, unityGain
		void join(float* const* samples, float gainOutDb,
			int numChannels, int numSamples, bool unityGain) noexcept
		{
			mixProcessor.join(samples, numChannels, numSamples, unityGain);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#else
		void join(ProcessorBufferView& view, float gainOutDb) noexcept
		{
			gainOut(view, gainOutDb);
		}
	#endif
#elif PPDIO == PPDIODryWet
	#if PPDIsNonlinear
		// samples, gainDryDb, gainWetInDb, numChannels, numSamples
		void split(float* const* samples, float gainDryDb, float gainWetInDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitNonlinear(samples, gainDryDb, gainWetInDb, numChannels, numSamples);
		}

		// samples, gainWetOutDb, gainOutDb, numChannels, numSamples, unityGain
		void join(float* const* samples, float gainWetOutDb, float gainOutDb,
			int numChannels, int numSamples, bool unityGain) noexcept
		{
			mixProcessor.joinNonlinear(samples, gainWetOutDb, numChannels, numSamples, unityGain);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#else
		// samples, gainDryDb, numChannels, numSamples
		void split(float* const* samples, float gainDryDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitLinear(samples, gainDryDb, numChannels, numSamples);
		}

		// samples, gainWetOutDb, gainOutDb, numChannels, numSamples
		void join(float* const* samples, float gainWetOutDb, float gainOutDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.joinLinear(samples, gainWetOutDb, numChannels, numSamples);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#endif
#else
	#if PPDIsNonlinear
		// view, gainWetInDb
		void split(ProcessorBufferView& view, float gainWetInDb) noexcept
		{
			mixProcessor.splitNonlinear(view, gainWetInDb);
		}

		// samples, mix, gainWetDb, gainOutDb, numChannels, numSamples, unityGain, delta
		void join(ProcessorBufferView& view, float mix,
			float gainWetDb, float gainOutDb, bool unityGain, bool delta) noexcept
		{
			mixProcessor.joinNonlinear(view, mix, gainWetDb, unityGain, delta);
			gainOut(view, gainOutDb);
		}
	#else
		// samples, numChannels, numSamples
		void split(float* const* samples,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitLinear(samples, numChannels, numSamples);
		}

		// samples, mix, gainWetDb, gainOutDb, numChannels, numSamples, delta
		void join(float* const* samples, float mix, float gainWetDb, float gainOutDb,
			int numChannels, int numSamples, bool delta) noexcept
		{
			mixProcessor.joinLinear(samples, mix, gainWetDb, numChannels, numSamples, delta);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#endif
#endif

	private:
#if PPDIO == PPDIOOut
		MixProcessorNonlinear mixProcessor;
#elif PPDIO == PPDIODryWet
		MixProcessorDryWet mixProcessor;
#else
		MixProcessorWetMix mixProcessor;
#endif
		Gain13 gainOut;
	};
}