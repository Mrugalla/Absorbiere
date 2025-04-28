#include "XenManager.h"
#include "Math.h"
#include <cmath>

namespace arch
{
	// Info

	bool XenManager::Info::operator==(const Info& x) const noexcept
	{
		return x.xen == xen && x.masterTune == masterTune &&
			x.anchor == anchor && x.pitchbendRange == pitchbendRange &&
			x.temperaments == temperaments;
	}

	template<typename Float>
	Float XenManager::Info::noteToFreqHz(Float note) const noexcept
	{
		// this still needs to be tested tho!
		const auto nFloor = std::floor(note);
		auto nI0 = static_cast<int>(nFloor);
		if (nI0 >= MaxXen)
			nI0 = MaxXen - 1;
		auto nI1 = nI0 + 1;
		if (nI1 >= MaxXen)
			nI1 = MaxXen - 1;
		const auto temp0 = static_cast<Float>(temperaments[nI0]);
		const auto temp1 = static_cast<Float>(temperaments[nI1]);
		const auto tempDiff = temp1 - temp0;
		const auto frac = note - nFloor;
		const auto temperamentAdjustment = temp0 + frac * tempDiff;
		return math::noteToFreqHz
		(
			note + temperamentAdjustment,
			static_cast<Float>(anchor),
			static_cast<Float>(xen),
			static_cast<Float>(masterTune)
		);
	}

	template<typename Float>
	Float XenManager::Info::freqHzToNote(Float hz) const noexcept
	{
		return math::freqHzToNote
		(
			hz,
			static_cast<Float>(anchor),
			static_cast<Float>(xen),
			static_cast<Float>(masterTune)
		);
	}

	// XenManager

	XenManager::XenManager() :
		info(),
		updateFunc([](const Info&, int){})
	{
		info.xen = 12.f;
		info.masterTune = 440.f;
		info.anchor = 69.f;
		info.pitchbendRange = 2.f;
		for (auto i = 0; i < MaxXen; ++i)
			info.temperaments[i] = 0.f;
	}

	void XenManager::operator()(const Info& nInfo, int numChannels) noexcept
	{
		if (info == nInfo)
			return;
		info = nInfo;
		updateFunc(info, numChannels);
	}

	template<typename Float>
	Float XenManager::noteToFreqHz(Float note) const noexcept
	{
		return info.noteToFreqHz(note);
	}

	template<typename Float>
	Float XenManager::noteToFreqHzWithWrap(Float note, Float lowestFreq, Float highestFreq) const noexcept
	{
		auto freq = noteToFreqHz(note);
		while (freq < lowestFreq)
			freq *= static_cast<Float>(2);
		while (freq >= highestFreq)
			freq *= static_cast<Float>(.5);
		return freq;
	}

	template<typename Float>
	Float XenManager::freqHzToNote(Float hz) const noexcept
	{
		return info.freqHzToNote(hz);
	}

	const XenManager::Info& XenManager::getInfo() const noexcept
	{
		return info;
	}

	/*
	double XenManager::getXen() const noexcept
	{
		return info.xen;
	}

	double XenManager::getPitchbendRange() const noexcept
	{
		return info.pitchbendRange;
	}

	double XenManager::getAnchor() const noexcept
	{
		return info.anchor;
	}

	double XenManager::getMasterTune() const noexcept
	{
		return info.masterTune;
	}
	*/


	template float XenManager::Info::noteToFreqHz<float>(float note) const noexcept;
	template double XenManager::Info::noteToFreqHz<double>(double note) const noexcept;

	template float XenManager::Info::freqHzToNote<float>(float hz) const noexcept;
	template double XenManager::Info::freqHzToNote<double>(double hz) const noexcept;

	template float XenManager::noteToFreqHz<float>(float note) const noexcept;
	template double XenManager::noteToFreqHz<double>(double note) const noexcept;

	template float XenManager::noteToFreqHzWithWrap<float>(float note, float lowestFreq, float highestFreq) const noexcept;
	template double XenManager::noteToFreqHzWithWrap<double>(double note, double lowestFreq, double highestFreq) const noexcept;

	template float XenManager::freqHzToNote<float>(float hz) const noexcept;
	template double XenManager::freqHzToNote<double>(double hz) const noexcept;
}