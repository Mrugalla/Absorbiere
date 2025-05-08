#pragma once
#include "DuckCompEditor.h"
#include "Oscilloscope.h"

namespace gui
{
	struct DuckEditor :
		public Comp
	{
		DuckEditor(Utils& u) :
			Comp(u),
			scope(u, u.audioProcessor.pluginProcessor.scope),
			rmBlend(u),
			compEditor(u),
			bpBlend(u),
			bpLow(u),
			bpHigh(u),
			bpListen(u)
		{
			layout.init
			(
				{ 1 },
				{ 1, 2, 3, 2 }
			);

			addAndMakeVisible(scope);
			addAndMakeVisible(rmBlend);
			addAndMakeVisible(compEditor);
			addAndMakeVisible(bpBlend);
			addAndMakeVisible(bpLow);
			addAndMakeVisible(bpHigh);
			addAndMakeVisible(bpListen);

			rmBlend.init(PID::DuckRingModBlend, "RM Blend");
			bpBlend.init(PID::DuckCompBPBlend, "BP Blend");
			bpLow.init(PID::DuckCompBPFreqLow, "BP Low");
			bpHigh.init(PID::DuckCompBPFreqHigh, "BP High");
			makeParameter(bpListen, PID::DuckCompBPListen, Button::Type::kToggle, "Listen");
		}

		void paint(Graphics& g) override
		{
			g.fillAll(getColour(CID::Darken));
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			layout.place(scope, 0, 0, 1, 1);
			layout.place(rmBlend, 0, 1, 1, 1);
			layout.place(compEditor, 0, 2, 1, 1);
			const auto bpArea = layout(0, 3, 1, 1);
			{
				const auto w = bpArea.getWidth() / 4.f;
				const auto h = bpArea.getHeight();
				const auto y = bpArea.getY();
				auto x = bpArea.getX();
				bpBlend.setBounds(BoundsF{ x, y, w, h }.toNearestInt());
				x += w;
				bpLow.setBounds(BoundsF{ x, y, w, h }.toNearestInt());
				x += w;
				bpHigh.setBounds(BoundsF{ x, y, w, h }.toNearestInt());
				x += w;
				bpListen.setBounds(BoundsF{ x, y, w, h }.toNearestInt());
			}
		}

	private:
		OscilloscopeEditor scope;
		KnobAbsorb rmBlend;
		DuckCompEditor compEditor;
		KnobAbsorb bpBlend, bpLow, bpHigh;
		Button bpListen;
	};
}