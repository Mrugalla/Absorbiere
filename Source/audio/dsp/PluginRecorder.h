#pragma once
#include "../Using.h"

namespace dsp
{
	struct PluginRecorder
	{
		PluginRecorder();

		// sampleRate
		void prepare(float);

		// samples, numChannels, numSamples
		void operator()(float* const*, int, int) noexcept;

		const AudioBuffer& getRecording();
	private:
		AudioBuffer recording, outBuffer;
		int writeHead;

		// samples, numChannels, numSamples
		void normalize(float* const*, int, int) noexcept;
	};
}


