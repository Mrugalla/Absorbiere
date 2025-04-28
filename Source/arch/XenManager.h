#pragma once
#include <array>
#include <atomic>
#include <functional>

namespace arch
{
	static constexpr int MaxXen = 48;
	static constexpr float MaxXenF = static_cast<float>(MaxXen);

	struct XenManager
	{
		struct Info
		{
			bool operator==(const Info&) const noexcept;

			template<typename Float>
			Float noteToFreqHz(Float) const noexcept;

			template<typename Float>
			Float freqHzToNote(Float) const noexcept;

			float xen, masterTune, anchor, pitchbendRange;
			std::array<float, MaxXen> temperaments;
		};

		// xenInfo, numChannels
		using UpdateFunc = std::function<void(const Info&, int)>;

		XenManager();

		// xenInfo, numChannels
		void operator()(const Info&, int) noexcept;

		template<typename Float>
		Float noteToFreqHz(Float) const noexcept;

		// note, lowestFreq, highestFreq
		template<typename Float>
		Float noteToFreqHzWithWrap(Float, Float = static_cast<Float>(0), Float = static_cast<Float>(22049)) const noexcept;

		template<typename Float>
		Float freqHzToNote(Float) const noexcept;

		const Info& getInfo() const noexcept;

		UpdateFunc updateFunc;
	protected:
		Info info;
	};
}