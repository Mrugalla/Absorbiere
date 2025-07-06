#pragma once
#include "Button.h"

namespace gui
{
	struct ButtonSCAutogain :
		public Comp
	{
		ButtonSCAutogain(Utils& u) :
			Comp(u),
			btn(u)
		{
			const auto fps = cbFPS::k15;
			add(Callback([&, speed = msToInc(5000.f, fps)]()
			{
				const auto listening = u.audioProcessor.scGainer.isListening();
				if (!listening)
					return;
				auto& phase = callbacks[0].phase;
				phase += speed;
				if (phase >= 1.f)
					--phase;
				repaint();
			}, 0, fps, true));

			addAndMakeVisible(btn);

			makePaintButton(btn, [](Graphics& g, const Button& b)
			{
				const auto hoverPhase = b.callbacks[Button::kHoverAniCB].phase;
				const auto toggleState = b.callbacks[Button::kToggleStateCB].phase;

				const auto thicc = b.utils.thicc;
				const auto thicc2 = thicc * 2.f;
				const auto thicc5 = thicc * 5.f;
				const auto thicc8 = thicc * 8.f;
				g.setColour(getColour(CID::Interact).interpolatedWith(juce::Colours::red, toggleState));
				const auto bounds = maxQuadIn(b.getLocalBounds());
				const auto minDimen = std::min(bounds.getWidth(), bounds.getHeight());
				const auto rad = minDimen * .5f;

				const auto margin = thicc5 - hoverPhase * thicc;
				const auto circleBounds = bounds.reduced(margin);
				g.drawEllipse(circleBounds, thicc);
				if (hoverPhase != 0.f)
				{
					const auto circleHoverBounds = bounds.reduced(rad + hoverPhase * (thicc8 - rad));
					g.drawEllipse(circleHoverBounds, thicc2);
				}
			}, "The plugin listens to the sidechain input to normalize it.");

			btn.type = Button::Type::kToggle;
			btn.onClick = [&](const Mouse&)
			{
				auto& scGainer = u.audioProcessor.scGainer;
				btn.value = scGainer.isListening() ? 0.f : 1.f;
				scGainer.setListening(btn.value);
			};
			btn.value = 0.f;
		}

		~ButtonSCAutogain()
		{
			utils.audioProcessor.scGainer.reset();
		}

		void paint(Graphics& g)
		{
			const auto thicc = utils.thicc;

			const auto loop = callbacks[0].phase;

			const auto w = static_cast<float>(getWidth());
			const auto h = static_cast<float>(getHeight());
			const auto bounds = maxQuadIn(getLocalBounds());
			const auto upscale = w > bounds.getWidth() ?
				w / bounds.getWidth() :
				h / bounds.getHeight();
			const auto boundsUpscale = bounds.expanded(upscale);
			const auto maxDimen = std::max(w, h);
			
			const auto numCircles = 7;
			const auto numCirclesF = static_cast<float>(numCircles);
			const auto numCirclesInv = 1.f / numCirclesF;
			for (auto i = 0; i < numCircles; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iNorm = iF * numCirclesInv;
				auto iShift = iNorm + loop;
				if (iShift >= 1.f)
					--iShift;
				g.setColour(getColour(CID::Interact).interpolatedWith(Colour(0x0), iShift));
				g.drawEllipse(boundsUpscale.reduced(iShift * maxDimen * .5f), thicc);
			}
		}

		void resized() override
		{
			Comp::resized();
			btn.setBounds(getLocalBounds());
		}

	private:
		Button btn;
	};
}