#include "Smooth.h"

namespace dsp
{
	namespace smooth
	{
		// Block
		Block::Block(float startVal) :
			curVal(startVal)
		{
		}

		/*
		void Block::operator()(float* bufferOut, float* bufferIn, int numSamples) noexcept
		{
			auto x = 0.f;
			const auto inc = 1.f / static_cast<float>(numSamples);

			for (auto s = 0; s < numSamples; ++s, x += inc)
			{
				const auto sIn = bufferIn[s];
				const auto sOut = bufferOut[s];
				bufferOut[s] = sIn + x * (sOut - sIn);
			}

			curVal = bufferOut[numSamples - 1];
		}
		*/

		void Block::operator()(float* buffer, float dest, int numSamples) noexcept
		{
			const auto dist = dest - curVal;
			const auto inc = dist / static_cast<float>(numSamples);

			for (auto s = 0; s < numSamples; ++s)
			{
				buffer[s] = curVal;
				curVal += inc;
			}
		}

		void Block::operator()(float* buffer, int numSamples) noexcept
		{
			SIMD::fill(buffer, curVal, numSamples);
		}

		// Lowpass static

		template<bool AutoGain>
		double Lowpass<AutoGain>::getXFromFc(double fc) noexcept
		{
			return std::exp(-TauD * fc);
		}

		template<bool AutoGain>
		double Lowpass<AutoGain>::getXFromHz(double hz, double Fs) noexcept
		{
			return getXFromFc(hz / Fs);
		}

		// Lowpass

		template<bool AutoGain>
		void Lowpass<AutoGain>::makeFromDecayInSamples(double d) noexcept
		{
			setX(std::exp(-1. / d));
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::makeFromDecayInSecs(double d, double Fs) noexcept
		{
			makeFromDecayInSamples(d * Fs);
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::makeFromDecayInFc(double fc) noexcept
		{
			setX(getXFromFc(fc));
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::makeFromDecayInHz(double hz, double Fs) noexcept
		{
			setX(getXFromHz(hz, Fs));
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::makeFromDecayInMs(double d, double Fs) noexcept
		{
			makeFromDecayInSamples(d * Fs * .001);
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::copyCutoffFrom(const Lowpass<AutoGain>& other) noexcept
		{
			a0 = other.a0;
			b1 = other.b1;
		}

		template<bool AutoGain>
		Lowpass<AutoGain>::Lowpass(double _startVal) :
			a0(1.),
			b1(0.),
			y1(_startVal),
			startVal(_startVal)
		{}

		template<bool AutoGain>
		void Lowpass<AutoGain>::reset()
		{
			reset(startVal);
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::reset(double v)
		{
			y1 = v;
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::operator()(double* buffer, double val, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = processSample(val);
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::operator()(double* buffer, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto y = processSample(buffer[s]);
				buffer[s] = y;
			}
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::operator()(float* buffer, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto y = processSample(buffer[s]);
				buffer[s] = static_cast<float>(y);
			}
		}

		template<bool AutoGain>
		double Lowpass<AutoGain>::operator()(double sample) noexcept
		{
			return processSample(sample);
		}

		template<bool AutoGain>
		double Lowpass<AutoGain>::processSample(double x0) noexcept
		{
			y1 = x0 * a0 + y1 * b1;
			return y1;
		}

		template<bool AutoGain>
		double Lowpass<AutoGain>::processSample(float x0) noexcept
		{
			return processSample(static_cast<double>(x0));
		}

		template<bool AutoGain>
		void Lowpass<AutoGain>::setX(double x) noexcept
		{
			a0 = 1. - x;

			if constexpr (AutoGain)
				b1 = x * (1. - a0);
			else
				b1 = x;
		}

		template struct Lowpass<true>;
		template struct Lowpass<false>;

		// Smooth

		void Smooth::makeFromDecayInMs(float smoothLenMs, float Fs) noexcept
		{
			lowpass.makeFromDecayInMs(static_cast<double>(smoothLenMs), static_cast<double>(Fs));
		}

		void Smooth::makeFromFreqInHz(float hz, float Fs) noexcept
		{
			lowpass.makeFromDecayInHz(static_cast<double>(hz), static_cast<double>(Fs));
		}

		Smooth::Smooth(float startVal) :
			block(startVal),
			lowpass(static_cast<double>(startVal)),
			cur(startVal),
			dest(startVal),
			smoothing(false)
		{
		}

		bool Smooth::operator()(float* bufferOut, float _dest, int numSamples) noexcept
		{
			dest = _dest;
			return operator()(bufferOut, numSamples);
		}

		bool Smooth::operator()(float* bufferOut, float _dest,
			int startIdx, int endIdx) noexcept
		{
			dest = _dest;
			auto numSamples = endIdx - startIdx;
			return operator()(&bufferOut[startIdx], numSamples);
		}

		/*
		void Smooth::operator()(float* bufferOut, float* bufferIn, int numSamples) noexcept
		{
			block(bufferOut, bufferIn, numSamples);
			lowpass(bufferOut, numSamples);
		}
		*/

		bool Smooth::operator()(float* bufferOut, int numSamples) noexcept
		{
			if (!smoothing && cur == dest)
				return false;

			smoothing = true;

			block(bufferOut, dest, numSamples);
			lowpass(bufferOut, numSamples);

			cur = bufferOut[numSamples - 1];
			const auto eps = 1e-6f;
			const auto dist = dest - cur;
			const auto distSquared = dist * dist;
			if (distSquared < eps)
			{
				smoothing = false;
				cur = dest;
			}
			return smoothing;
		}

		float Smooth::operator()(float _dest) noexcept
		{
			return static_cast<float>(lowpass(_dest));
		}
	}
}