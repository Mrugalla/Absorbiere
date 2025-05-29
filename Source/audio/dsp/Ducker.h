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

		// things to try:
		// 2 pole smoother for attack and release
		// rms based level detection instead of abs
		// lookahead

		struct CompChatGPT
		{
			CompChatGPT() :
				gainSmooth(0.f)
			{ }

			void prepare(double _sampleRate) noexcept
			{
				sampleRate = _sampleRate;
			}

			void setLevelDetector(double _atkMs, double _rlsMs) noexcept
			{
				atkMs = _atkMs;
				rlsMs = _rlsMs;
				atk = std::exp(-1. / (.001 * atkMs * sampleRate));
				rls = std::exp(-1. / (.001 * rlsMs * sampleRate));
			}

			void copyLevelDetector(const CompChatGPT& other) noexcept
			{
				atkMs = other.atkMs;
				rlsMs = other.rlsMs;
				atk = other.atk;
				rls = other.rls;
			}

			void operator()(float* smplsMain, const float* smplsSC,
				float thresholdDb, float ratioDb, float kneeDb, float blend,
				int numSamples) noexcept
			{
				for (auto s = 0; s < numSamples; ++s)
				{
					auto y = process(smplsMain[s], smplsSC[s], thresholdDb, ratioDb, kneeDb);
					smplsMain[s] = smplsMain[s] + blend * (y - smplsMain[s]);
				}
					
			}

			PointF getMeter(float thresholdDb, float ratioDb, float kneeDb) const noexcept
			{
				return { gainSmooth, computeGainReduction(gainSmooth, thresholdDb, ratioDb, kneeDb) };
			}
		private:
			double sampleRate, atkMs, rlsMs, atk, rls;
			float gainSmooth;

			float process(float main, float sc, float thresholdDb, float ratioDb, float kneeDb) noexcept
			{
				const auto xDb = math::ampToDecibel(std::fabs(sc) + 1e-6f);
				const auto gainDb = computeGainReduction(xDb, thresholdDb, ratioDb, kneeDb);
				const auto targetGain = math::dbToAmp(-gainDb);
				const auto dist = static_cast<double>(gainSmooth - targetGain);
				if (targetGain < gainSmooth)
					gainSmooth = targetGain + static_cast<float>(atk * dist);
				else
					gainSmooth = targetGain + static_cast<float>(rls * dist);
				return main * gainSmooth;
			}

			float computeGainReduction(float xDb, float thresholdDb, float ratioDb, float kneeDb) const noexcept
			{
				const auto overThreshold = xDb - thresholdDb;

				if (kneeDb > 0.f)
				{
					const auto halfKnee = kneeDb * .5f;
					if (overThreshold < -halfKnee)
						return 0.f;
					else if (overThreshold <= halfKnee) {
						const auto x = (overThreshold + halfKnee) / kneeDb;
						const auto softGain = ((1.f / ratioDb - 1.f) * x * x) / 2.f;
						return -softGain * overThreshold;
					}
				}

				return overThreshold > 0.f ? overThreshold * (1.f - 1.f / ratioDb) : 0.f;
			}
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
			std::array<CompChatGPT, 2> compressors;
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
		Ducker()
		{}

		void operator()(ProcessorBufferView& view) noexcept
		{
			if (!view.scEnabled)
				return;
			applyRingMod(view);
		}
	private:
		void applyRingMod(ProcessorBufferView& view)
		{
			const auto numSamples = view.numSamples;
			const auto numChannelsMain = view.getNumChannelsMain();
			for (auto ch = 0; ch < numChannelsMain; ++ch)
			{
				const auto smplsSC = view.getSamplesSC(ch);
				auto smplsMain = view.getSamplesMain(ch);
				
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto smplRect = std::abs(smplsSC[s]);
					const auto smplPol = -smplRect;
					const auto x = smplsMain[s];
					const auto ringModSmpl = x * smplPol;
					const auto y = x + ringModSmpl;
					smplsMain[s] = y;
				}
			}
		}
	};
}