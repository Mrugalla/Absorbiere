#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../arch/Math.h"
#include <array>

namespace dsp
{
	using MidiBuffer = juce::MidiBuffer;
	using MidiMessage = juce::MidiMessage;
	using SIMD = juce::FloatVectorOperations;
	using AudioBuffer = juce::AudioBuffer<float>;
	using Uint8 = uint8_t;
	using Int64 = juce::int64;
	using PlayHead = juce::AudioPlayHead;
	using PosInfo = PlayHead::CurrentPositionInfo;
	using MemoryInputStream = juce::MemoryInputStream;
	using WavAudioFormat = juce::WavAudioFormat;
	using AudioFormatReader = juce::AudioFormatReader;
	using FileOutputStream = juce::FileOutputStream;
	using PNGImageFormat = juce::PNGImageFormat;
	using File = juce::File;
	using Image = juce::Image;
	using Graphics = juce::Graphics;
	using String = juce::String;
	using BoundsF = juce::Rectangle<float>;
	using Random = juce::Random;
	using PointF = juce::Point<float>;

	static constexpr float Pi = 3.1415926535897932384626433832795f;
	static constexpr float Tau = Pi * 2.f;
	static constexpr float PiHalf = Pi * .5f;
	static constexpr float E = 2.7182818284590452353602874713527f;
	static constexpr float TauInv = 1.f / Tau;
	static constexpr float SixtyInv = 1.f / 60.f;
	static constexpr double PiD = 3.1415926535897932384626433832795;
	static constexpr double TauD = PiD * 2.;

	static constexpr int NumMIDIChannels = 16;
	static constexpr int NumMPEChannels = NumMIDIChannels - 1;
	static constexpr float PitchbendRange = 16383.f;
	static constexpr float PitchbendCenter = PitchbendRange * .5f;

	static constexpr int NumChannels = 2;
    static constexpr int BlockSize = 32;

	static constexpr auto BlockSizeF = static_cast<float>(BlockSize);
	static constexpr auto BlockSizeD = static_cast<double>(BlockSize);

	using BufferView2 = std::array<float*, 2>;
	using BufferView3 = std::array<float*, 3>;
	using BufferView4 = std::array<float*, 4>;

	struct BufferView2X
	{
		BufferView2X() :
			view(),
			numChannels(0)
		{}

		void assign(float* const* samples, int _numChannels) noexcept
		{
			for (auto ch = 0; ch < _numChannels; ++ch)
			{
				auto smpls = samples[ch];
				view[ch] = smpls;
			}
			numChannels = _numChannels;
		}

		void applyGain(float gain, int numSamples) noexcept
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view[ch], gain, numSamples);
		}

		const float* operator[](int ch) const noexcept
		{
			return view[ch];
		}

		float* operator[](int ch) noexcept
		{
			return view[ch];
		}

		BufferView2 view;
		int numChannels;
	};

	struct ProcessorBufferView
	{
		ProcessorBufferView() :
			main(),
			sc(),
			numSamples(0),
			scEnabled(false)
		{
		}

		void assignMain(float* const* samples, int numChannels, int _numSamples) noexcept
		{
			main.assign(samples, numChannels);
			numSamples = _numSamples;
			scEnabled = false;
		}

		void assignSC(float* const* samples, float scGain,
			int numChannels, bool listenSC) noexcept
		{
			sc.assign(samples, numChannels);
			if (scGain == 0.f)
			{
				scEnabled = false;
				return;
			}
			if (scGain != 1.f)
				sc.applyGain(scGain, numSamples);
			if (listenSC)
			{
				for (auto ch = 0; ch < main.numChannels; ++ch)
				{
					auto smplsMain = main.view[ch];
					auto chSC = ch;
					if (chSC >= sc.numChannels)
						chSC -= sc.numChannels;
					const auto smplsSC = sc.view[chSC];
					SIMD::copy(smplsMain, smplsSC, numSamples);
				}
			}
			scEnabled = !listenSC;
		}

		void useMainForSCIfRequired() noexcept
		{
			if (scEnabled)
				return;
			for (auto ch = 0; ch < main.numChannels; ++ch)
				sc.view[ch] = main.view[ch];
			sc.numChannels = main.numChannels;
		}

		void fillBlock(ProcessorBufferView& buffer, int s) noexcept
		{
			const auto dif = buffer.numSamples - s;
			numSamples = dif < dsp::BlockSize ? dif : dsp::BlockSize;
			main.numChannels = buffer.main.numChannels;
			for (auto ch = 0; ch < main.numChannels; ++ch)
				main.view[ch] = &buffer.main.view[ch][s];
			sc.numChannels = buffer.sc.numChannels;
			for (auto ch = 0; ch < sc.numChannels; ++ch)
				sc.view[ch] = &buffer.sc.view[ch][s];
			scEnabled = buffer.scEnabled;
		}

		BufferView2X getViewMain() noexcept
		{
			return main;
		}

		BufferView2X getViewSC() noexcept
		{
			return sc;
		}

		float* getSamplesMain(int ch) noexcept
		{
			return main.view[ch];
		}

		const float* getSamplesMain(int ch) const noexcept
		{
			return main.view[ch];
		}

		float* getSamplesSC(int ch) noexcept
		{
			return sc.view[ch];
		}

		const float* getSamplesSC(int ch) const noexcept
		{
			return sc.view[ch];
		}

		int getNumChannelsMain() const noexcept
		{
			return main.numChannels;
		}

		int getNumChannelsSC() const noexcept
		{
			return sc.numChannels;
		}

		int getNumSamples() const noexcept
		{
			return numSamples;
		}

		BufferView2X main, sc;
		int numSamples;
		bool scEnabled;
	};

	// returns the sampleRate of the wav file
	inline double loadFromMemory(AudioBuffer& buffer, const char* data, int size)
	{
		double sampleRate = 0.;
		auto stream = new MemoryInputStream(data, size, false);
		WavAudioFormat wav;
		const auto reader = wav.createReaderFor(stream, false);
		if (reader != nullptr)
		{
			sampleRate = reader->sampleRate;
			const auto numChannels = static_cast<int>(reader->numChannels);
			const auto numSamples = static_cast<int>(reader->lengthInSamples);
			buffer.setSize(numChannels, numSamples, false, false, false);
			reader->read(&buffer, 0, numSamples, 0, true, true);
		}
		delete reader;
		return sampleRate;
	}
}