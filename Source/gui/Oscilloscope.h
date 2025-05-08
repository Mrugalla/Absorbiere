#pragma once
#include "../audio/dsp/Oscilloscope.h"
#include "Button.h"

namespace gui
{
	struct Oscilloscope :
		public Comp
	{
		using Oscope = dsp::Oscilloscope;

		Oscilloscope(Utils& u, const Oscope& _oscope) :
			Comp(u),
			oscope(_oscope),
			curve(),
			bipolar(false)
		{
			add(Callback([&]()
			{
				repaint();
			}, 0, cbFPS::k30, true));
		}

		void resized() override
		{
			const auto thicc = utils.thicc;
			bounds = getLocalBounds().toFloat().reduced(thicc);
			curve.clear();
			curve.preallocateSpace(static_cast<int>(bounds.getWidth()) + 1);
		}

		void paint(Graphics& g) override
		{
			const auto thicc = utils.thicc;
			Stroke stroke(thicc, Stroke::JointStyle::beveled, Stroke::EndCapStyle::rounded);

			setCol(g, CID::Darken);
			g.fillRoundedRectangle(bounds, thicc);

			const auto data = oscope.data();
			const auto size = oscope.windowLength();
			const auto sizeF = static_cast<float>(size);
			const auto beatLength = oscope.getBeatLength();
			const auto w = bounds.getWidth();
			const auto h = bounds.getHeight();
			const auto xScale = w / std::min(beatLength, sizeF);
			const auto bipolarVal = bipolar ? 1.f : 0.f;
			const auto yScale = h - bipolarVal * (h * .5f - h);
			const auto xScaleInv = 1.f / xScale;
			const auto xOff = bounds.getX();
			const auto yOff = bounds.getY() + yScale;

			curve.clear();
			auto y = yOff - data[0] * yScale;
			curve.startNewSubPath(xOff, y);
			for (auto i = thicc; i <= w; i += thicc)
			{
				const auto x = xOff + i;
				const auto idx = static_cast<int>(i * xScaleInv);
				y = yOff - data[idx] * yScale;
				curve.lineTo(x, y);
			}

			setCol(g, CID::Txt);
			g.strokePath(curve, stroke);
		}

	protected:
		const Oscope& oscope;
		BoundsF bounds;
		Path curve;
	public:
		bool bipolar;
	};
}