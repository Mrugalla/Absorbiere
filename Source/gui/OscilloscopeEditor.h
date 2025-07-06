#pragma once
#include "../audio/dsp/Oscilloscope.h"
#include "Button.h"

namespace gui
{
	struct OscilloscopeEditor :
		public Button
	{
		using Oscope = dsp::Oscilloscope;

		OscilloscopeEditor(Utils& u, Oscope& _oscope) :
			Button(u),
			oscope(_oscope),
			curve(),
			bipolar(false)
		{
			tooltip = "Click on the scope to switch between sc input and main output";
			onClick = [&](const Mouse&)
			{
				_oscope.setSCEnabled(!oscope.isSCEnabled());
			};

			add(Callback([&]()
			{
				repaint();
			}, 0, cbFPS::k30, true));
		}

		void resized() override
		{
			Comp::resized();
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
			auto idx0 = 0;
			auto y = yOff - data[idx0] * yScale;
			curve.startNewSubPath(xOff, y);
			for (auto i = thicc; i <= w; i += thicc)
			{
				const auto x = xOff + i;
				const auto idx1 = static_cast<int>(i * xScaleInv);
		
				auto smpl = 0.f;
				for (auto j = idx0; j < idx1; ++j)
				{
					const auto rect = data[j] * data[j];
					if (smpl < rect)
						smpl = rect;
				}
				idx0 = idx1;
				smpl = std::sqrt(smpl);
				y = yOff - smpl * yScale;
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