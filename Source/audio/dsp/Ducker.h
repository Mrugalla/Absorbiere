#pragma once
#include "Smooth.h"
#include "AbsorbiereAxiom.h"

namespace dsp
{
	struct Ducker
	{
	private:
		static constexpr float MinDb = -120.f;
		using BufferSC = std::array<std::array<float, BlockSize>, 2>;

		struct Params
		{
			Params() :
				rmBlend(0.f),
				compBlend(0.f), compThreshold(0.f), compRatio(0.f), compKnee(0.f), compAtk(0.f), compRls(0.f),
				compBPBlend(0.f), compBPFreqLow(0.f), compBPFreqHigh(0.f),
				compBPListen(false)
			{}

			float rmBlend,
				compBlend, compThreshold, compRatio, compKnee, compAtk, compRls,
				compBPBlend, compBPFreqLow, compBPFreqHigh;
			bool compBPListen;
		};

		struct SidechainFilter
		{
			SidechainFilter() :
				lows(),
				highs(),
				sampleRate(1.)
			{}

			void prepare(double _sampleRate) noexcept
			{
				sampleRate = _sampleRate;
				for (auto& low : lows)
					low.reset();
				for (auto& high : highs)
					high.reset();
			}

			void setRange(double hzLow, double hzHigh, int numChannels) noexcept
			{
				for (auto ch = 0; ch < numChannels; ++ch)
				{
					for (auto i = 0; i < 2; ++i)
					{
						const auto ch2 = ch + i * 2;
						lows[ch2].makeFromDecayInHz(hzLow, sampleRate);
						highs[ch2].makeFromDecayInHz(hzHigh, sampleRate);
					}
				}
			}

			void operator()(float* smpls, float blend, int numSamples, int ch) noexcept
			{
				const auto ch2 = ch + 2;
				auto& low0 = lows[ch];
				auto& high0 = highs[ch];
				auto& low1 = lows[ch2];
				auto& high1 = highs[ch2];
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto x = smpls[s];
					const auto withUpperLimit = high1(high0(x));
					const auto withLowerLimit = static_cast<float>(withUpperLimit - low1(low0(withUpperLimit)));
					const auto y = x + blend * (withLowerLimit - x);
					smpls[s] = y;
				}
			}
		private:
			std::array<smooth::LowpassG0, 4> lows, highs;
			double sampleRate;
		};

		struct LevelDetector
		{
			LevelDetector() :
				lp(MinDb),
				sampleRate(1.f),
				atkX(0.f),
				rlsX(0.f),
				env(MinDb)
			{
			}

			void prepare(double _sampleRate) noexcept
			{
				sampleRate = _sampleRate;
			}

			void operator()(double atkMs, double rlsMs) noexcept
			{
				atkX = lp.getXFromMs(atkMs, sampleRate);
				rlsX = lp.getXFromMs(rlsMs, sampleRate);
			}

			void copyFrom(const LevelDetector& other) noexcept
			{
				lp.copyCutoffFrom(other.lp);
				atkX = other.atkX;
				rlsX = other.rlsX;
			}

			float operator()(float rectDb) noexcept
			{
				if (env < rectDb)
					lp.setX(atkX);
				else
					lp.setX(rlsX);

				env = static_cast<float>(lp(rectDb));

				return env;
			}
		protected:
			smooth::LowpassG0 lp;
			double sampleRate, atkX, rlsX;
		public:
			float env;
		};

		struct Compressor
		{
			Compressor() :
				lvlDetector()
			{}

			void prepare(double sampleRate)
			{
				lvlDetector.prepare(sampleRate);
			}

			void setLevelDetector(double atkMs, double rlsMs) noexcept
			{
				lvlDetector(atkMs, rlsMs);
			}

			void copyLevelDetector(const Compressor& other) noexcept
			{
				lvlDetector.copyFrom(other.lvlDetector);
			}

			void operator()(float* smplsMain, const float* smplsSC,
				float thresholdDb, float ratioDb, float kneeDb, float blend,
				int numSamples) noexcept
			{
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto x = smplsMain[s];
					const auto sc = smplsSC[s];
					const auto rect = std::abs(sc);
					const auto rectDb = rect == 0.f ? MinDb : math::ampToDecibel(rect);
					const auto tc = TransferDownwardsComp(rectDb, thresholdDb, ratioDb, kneeDb);
					const auto gainDb = tc - rectDb;
					const auto lvl = lvlDetector(gainDb);
					const auto gain = math::dbToAmp(lvl);
					const auto y = x * gain;
					smplsMain[s] = x + blend * (y - x);
				}
			}

			PointF getMeter(float thresholdDb, float ratioDb, float kneeDb) const noexcept
			{
				return { lvlDetector.env, TransferDownwardsComp(lvlDetector.env, thresholdDb, ratioDb, kneeDb) };
			}
		private:
			LevelDetector lvlDetector;
		};

		struct CompressorStereo
		{
			CompressorStereo() :
				compressors(),
				atkMs(1.),
				rlsMs(1.),
				meter({ 0.f, 0.f })
			{
			}

			void prepare(double sampleRate)
			{
				for (auto& comp : compressors)
					comp.prepare(sampleRate);
			}

			void setAttack(double _atkMs, int numChannels) noexcept
			{
				if (atkMs == _atkMs)
					return;
				atkMs = _atkMs;
				updateLevelDetector(numChannels);
			}

			void setRelease(double _rlsMs, int numChannels) noexcept
			{
				if (rlsMs == _rlsMs)
					return;
				rlsMs = _rlsMs;
				updateLevelDetector(numChannels);
			}

			void operator()(BufferView2X mainBuf, const BufferSC& scBuf,
				float thresholdDb, float ratioDb, float kneeDb, float blend,
				int numSamples) noexcept
			{
				for (auto ch = 0; ch < mainBuf.numChannels; ++ch)
				{
					auto& comp = compressors[ch];
					auto main = mainBuf[ch];
					const auto sc = scBuf[ch].data();
					comp.operator()(main, sc, thresholdDb, ratioDb, kneeDb, blend, numSamples);
				}

				PointF m(0.f, 0.f);
				for (auto ch = 0; ch < mainBuf.numChannels; ++ch)
					m += compressors[ch].getMeter(thresholdDb, ratioDb, kneeDb);
				meter.store(m / static_cast<float>(mainBuf.numChannels));
			}

			const std::atomic<PointF>& getMeter() const noexcept
			{
				return meter;
			}
		private:
			std::array<Compressor, 2> compressors;
			double atkMs, rlsMs;
			std::atomic<PointF> meter;

			void updateLevelDetector(int numChannels) noexcept
			{
				compressors[0].setLevelDetector(atkMs, rlsMs);
				if (numChannels > 1)
					compressors[1].copyLevelDetector(compressors[0]);
			}
		};
	public:
		Ducker() :
			params(),
			bufferSC(),
			sidechainFilter(),
			compressor(),
			sampleRate(1.)
		{}

		void setRMBlend(float e) noexcept
		{
			params.rmBlend = e;
		}

		void setCompBlend(float e) noexcept
		{
			params.compBlend = e;
		}

		void setCompThreshold(float e) noexcept
		{
			params.compThreshold = e;
		}

		void setCompRatio(float e) noexcept
		{
			params.compRatio = e;
		}

		void setCompKnee(float e) noexcept
		{
			params.compKnee = e;
		}

		void setCompAtk(float e, int numChannels) noexcept
		{
			params.compAtk = e;
			compressor.setAttack(e, numChannels);
		}

		void setCompRls(float e, int numChannels) noexcept
		{
			params.compRls = e;
			compressor.setRelease(e, numChannels);
		}

		void setCompBPFreqLow(float e, int numChannels) noexcept
		{
			params.compBPFreqLow = e;
			updateFrequencies(numChannels);
		}

		void setCompBPFreqHigh(float e, int numChannels) noexcept
		{
			params.compBPFreqHigh = e;
			updateFrequencies(numChannels);
		}

		void setCompBPBlend(float e)
		{
			params.compBPBlend = e;
		}

		void setCompBPListen(bool e) noexcept
		{
			params.compBPListen = e;
		}

		void prepare(double _sampleRate) noexcept
		{
			sampleRate = _sampleRate;
			compressor.prepare(sampleRate);
			sidechainFilter.prepare(sampleRate);
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			if (!view.scEnabled)
				return;
			copySCBuffer(view);
			applyRingMod(view);
			filterSC(view);
			if (params.compBPListen)
				return listenSC(view);
			applyCompressor(view);
		}

		const std::atomic<PointF>& getMeter() const noexcept
		{
			return compressor.getMeter();
		}
	private:
		Params params;
		BufferSC bufferSC;
		SidechainFilter sidechainFilter;
		CompressorStereo compressor;
		double sampleRate;

		void updateFrequencies(int numChannels) noexcept
		{
			auto f0 = params.compBPFreqLow;
			auto f1 = params.compBPFreqHigh;
			if (f0 > f1)
				std::swap(f0, f1);
			sidechainFilter.setRange(f0, f1, numChannels);
		}

		void copySCBuffer(ProcessorBufferView& view) noexcept
		{
			const auto numSamples = view.numSamples;
			const auto numChannelsMain = view.getNumChannelsMain();
			const auto numChannelsSC = view.getNumChannelsSC();
			for (auto ch = 0; ch < numChannelsMain; ++ch)
			{
				auto chSC = ch;
				if (chSC >= numChannelsSC)
					chSC = chSC - numChannelsSC;
				auto smplsSC = view.getSamplesSC(chSC);
				auto smplsBuf = bufferSC[ch].data();
				SIMD::copy(smplsBuf, smplsSC, numSamples);
			}
		}
		
		void applyRingMod(ProcessorBufferView& view)
		{
			const auto numSamples = view.numSamples;
			const auto numChannelsMain = view.getNumChannelsMain();
			for (auto ch = 0; ch < numChannelsMain; ++ch)
			{
				const auto smplsSC = bufferSC[ch].data();
				auto smplsMain = view.getSamplesMain(ch);
				
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto smplRect = std::abs(smplsSC[s]);
					const auto smplPol = -smplRect;
					const auto x = smplsMain[s];
					const auto ringModSmpl = x * smplPol;
					const auto y = x + ringModSmpl;
					smplsMain[s] = x + params.rmBlend * (y - x);
				}
			}
		}

		void filterSC(ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smplsSC = bufferSC[ch].data();
				sidechainFilter(smplsSC, params.compBPBlend, view.numSamples, ch);
			}
		}

		void listenSC(ProcessorBufferView& view) noexcept
		{
			const auto numSamples = view.numSamples;
			const auto numChannelsMain = view.getNumChannelsMain();
			for (auto ch = 0; ch < numChannelsMain; ++ch)
			{
				const auto smplsSC = bufferSC[ch].data();
				auto smplsMain = view.getSamplesMain(ch);
				SIMD::copy(smplsMain, smplsSC, numSamples);
			}
		}

		void applyCompressor(ProcessorBufferView& view)
		{
			compressor
			(
				view.getViewMain(),
				bufferSC,
				params.compThreshold,
				params.compRatio,
				params.compKnee,
				params.compBlend,
				view.numSamples
			);
		}
	};
}