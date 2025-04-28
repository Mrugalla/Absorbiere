#include "PRM.h"

namespace dsp
{
	// PRMInfo

	PRMInfo::PRMInfo(float* _buf, float _val, bool _smoothing) :
		buf(_buf),
		val(_val),
		smoothing(_smoothing)
	{}

	float PRMInfo::operator[](int i) const noexcept
	{
		return buf[i];
	}

	void PRMInfo::copyToBuffer(int numSamples) noexcept
	{
		if (smoothing)
			return;
		SIMD::fill(buf, val, numSamples);
	}

	void PRMInfo::copyToBuffer(int startIdx, int endIdx) noexcept
	{
		if (smoothing)
			return;
		const auto numSamples = endIdx - startIdx;
		SIMD::fill(&buf[startIdx], val, numSamples);
	}

	PRMInfo::operator float() const noexcept
	{
		return val;
	}

	// PRM
	
	PRM::PRM(float startVal) :
		buf(),
		smooth(startVal),
		value(startVal),
		smoothing(false)
	{
		for(auto& b : buf)
			b = startVal;
	}
	
	void PRM::prepare(float sampleRate, float smoothLenMs) noexcept
	{
		smooth.makeFromDecayInMs(smoothLenMs, sampleRate);
		smoothing = false;
	}

	PRMInfo PRM::operator()(float val, int numSamples) noexcept
	{
		value = val;
		smoothing = smooth(buf.data(), value, numSamples);
		return { buf.data(), value, smoothing };
	}

	PRMInfo PRM::operator()(float val, int startIdx, int endIdx) noexcept
	{
		value = val;
		return operator()(startIdx, endIdx);
	}

	PRMInfo PRM::operator()(int startIdx, int endIdx) noexcept
	{
		smoothing = smooth(buf.data(), value, startIdx, endIdx);
		return { buf.data(), value, smoothing };
	}

	PRMInfo PRM::operator()(int numSamples) noexcept
	{
		smoothing = smooth(buf.data(), value, numSamples);
		return { buf.data(), value, smoothing };
	}

	float PRM::operator[](int i) const noexcept
	{
		return buf[i];
	}

	// PRMBlock

	PRMBlock::PRMBlock(float _startVal) :
		startVal(_startVal),
		lp(static_cast<double>(startVal)),
		info(nullptr, startVal, false)
	{}

	void PRMBlock::prepare(float sampleRate, float smoothLenMs) noexcept
	{
		const auto smoothLenBlock = smoothLenMs / BlockSizeF;
		lp.makeFromDecayInMs(smoothLenBlock, sampleRate);
	}

	void PRMBlock::reset() noexcept
	{
		reset(startVal);
	}

	void PRMBlock::reset(float val) noexcept
	{
		lp.reset(static_cast<double>(val));
		info.val = val;
	}
	
	PRMInfo PRMBlock::operator()(float x) noexcept
	{
		if (info.val != x)
		{
			info.smoothing = true;
			const auto lpY = lp(x);
			info.val = static_cast<float>(lpY);
			if (info.val == x)
				info.smoothing = false;
		}
		return info;
	}
	
	PRMBlock::operator float() const noexcept
	{
		return info;
	}

	//
	
	PRMBlockStereo::PRMBlockStereo(float startVal) :
		prms{ startVal, startVal }
	{}
	
	void PRMBlockStereo::prepare(float sampleRate, float smoothLenMs) noexcept
	{
		for (auto& prm : prms)
			prm.prepare(sampleRate, smoothLenMs);
	}

	PRMInfo PRMBlockStereo::operator()(float x, int ch) noexcept
	{
		return prms[ch](x);
	}

	PRMInfo PRMBlockStereo::operator[](int ch) const noexcept
	{
		return prms[ch].info;
	}
}