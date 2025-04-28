#pragma once
#include "../Using.h"

namespace dsp
{
    template<size_t NumTracks, bool Smooth>
    struct XFadeMixer
    {
        struct Track
        {
            Track() :
                gain(0.),
                destGain(0.),
                inc(0.),
                fading(false)
            {
            }

            void prepare(double _inc)
            {
				gain = 0.;
                inc = _inc;
            }

            void disable() noexcept
            {
                destGain = 0.;
            }

            void enable() noexcept
            {
                destGain = 1.;
            }

            const bool isEnabled() const noexcept
            {
                return gain + destGain != 0.;
            }

            const bool isFading() const noexcept
            {
                return destGain != gain;
            }

            void synthesizeGainValues(float* xBuf, int numSamples) noexcept
            {
                if (!isFading())
                {
                    SIMD::fill(xBuf, static_cast<float>(gain), numSamples);
                    fading = false;
                    return;
                }

                fading = true;
                synthesizeGainValuesInternal(xBuf, numSamples);

                if (Smooth)
                    makeSmooth(xBuf, numSamples);
            }

            void copy(float* dest, const float* src,
                const float* gainBuf, int numSamples) const noexcept
            {
                if (fading)
                    SIMD::multiply(dest, src, gainBuf, numSamples);
                else if (gain == 1.)
                    SIMD::copy(dest, src, numSamples);
            }

            void add(float* dest, const float* src,
                const float* gainBuf, int numSamples) const noexcept
			{
				if (fading)
					SIMD::addWithMultiply(dest, src, gainBuf, numSamples);
				else if (gain == 1.)
					SIMD::add(dest, src, numSamples);
			}

            double gain, destGain, inc;
        protected:
            bool fading;

            void synthesizeGainValuesInternal(float* xBuf, int numSamples) noexcept
            {
                if (destGain == 1.)
                {
                    for (auto s = 0; s < numSamples; ++s)
                    {
                        xBuf[s] = static_cast<float>(gain);
                        gain += inc;
                        if (gain >= 1.)
                        {
                            gain = 1.;
                            for (; s < numSamples; ++s)
                                xBuf[s] = static_cast<float>(gain);
                            return;
                        }
                    }
                    return;
                }
                for (auto s = 0; s < numSamples; ++s)
                {
                    xBuf[s] = static_cast<float>(gain);
                    gain -= inc;
                    if (gain < 0.)
                    {
                        gain = 0.;
                        for (; s < numSamples; ++s)
                            xBuf[s] = static_cast<float>(gain);
                        return;
                    }
                }
            }

            void makeSmooth(float* xBuf, int numSamples) const noexcept
            {
                for (auto s = 0; s < numSamples; ++s)
                    xBuf[s] = std::cos(xBuf[s] * Pi + Pi) * .5f + .5f;
            }
        };

        XFadeMixer() :
            buffer(),
            tracks(),
            idx(0)
        {
        }

        void prepare(float sampleRate, float lengthMs)
        {
            for(auto& b: buffer)
				for (auto& x : b)
					x = 0.f;
            const auto inc = static_cast<double>(math::msToInc(lengthMs, sampleRate));
            for (auto& track : tracks)
                track.prepare(inc);
            tracks[idx].gain = 1.;
        }

        BufferView2 operator()(int i, int numSamples) noexcept
        {
            auto& track = tracks[i];
            const auto idx3 = i * 3;
			auto xBuf = buffer[idx3 + 2].data();
			track.synthesizeGainValues(xBuf, numSamples);
			return { buffer[idx3].data(), buffer[idx3 + 1].data() };
        }

        void copy(float* smpls, int i, int numSamples)
        {
			auto& track = tracks[i];
			const auto i3 = i * 3;
			track.copy(smpls, buffer[i3].data(), buffer[i3 + 2].data(), numSamples);
        }

        void add(float* smpls, int i, int numSamples)
        {
            auto& track = tracks[i];
            const auto i3 = i * 3;
            track.add(smpls, buffer[i3].data(), buffer[i3 + 2].data(), numSamples);
        }

        void init() noexcept
        {
            ++idx;
            if (idx >= NumTracks)
                idx = 0;
            for (auto& track : tracks)
                track.disable();
            tracks[idx].enable();
        }

        /*
        BufferView3 getBuffer(int i) noexcept
        {
			return { buffer[i * 3].data(), buffer[i * 3 + 1].data(), buffer[i * 3 + 2].data() };
        }
        */

        const int numTracksEnabled() const noexcept
        {
            auto sum = 0;
            for (auto i = 0; i < NumTracks; ++i)
                sum += (tracks[i].isEnabled() ? 1 : 0);
            return sum;
        }

        Track& operator[](int i) noexcept
        {
            return tracks[i];
        }

        bool stillFading() const noexcept
        {
            return tracks[idx].gain != 1.;
        }

    protected:
        std::array<std::array<float, BlockSize>, NumTracks * 3> buffer;
        std::array<Track, NumTracks> tracks;
    public:
        int idx;
    };

}