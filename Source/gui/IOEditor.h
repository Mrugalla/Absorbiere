#pragma once
#include "KnobAbsorb.h"
#include "ColoursEditor.h"
#include "ButtonSCAutogain.h"
#include "ButtonPower.h"

namespace gui
{
	struct IOEditor :
		public Comp
	{
		IOEditor(Utils& u) :
			Comp(u),
			macro(u),
			scGain(u),
			scListen(u),
			gainIn(u),
			scAuto(u),
			unityGain(u),
			gainWet(u),
			mix(u),
			gainOut(u),
			power(u),
			coloursEditor(u),
			buttonColours(coloursEditor)
		{
			layout.init
			(
				{ 1, 1, 1 },
				{ 8, 2, 1, 1 }
			);

			addAndMakeVisible(macro);
			addAndMakeVisible(scGain);
			addAndMakeVisible(scAuto);
			addAndMakeVisible(scListen);
			addAndMakeVisible(gainIn);
			addAndMakeVisible(unityGain);
			addAndMakeVisible(gainWet);
			addAndMakeVisible(mix);
			addAndMakeVisible(gainOut);
			addAndMakeVisible(power);
			addChildComponent(coloursEditor);
			addAndMakeVisible(buttonColours);

			macro.init(PID::Macro, "Macro");
			scGain.init(PID::SCGain, "SC Gain");
			gainIn.init(PID::GainIn, "Gain In");
			gainWet.init(PID::GainWet, "Gain Wet");
			mix.init(PID::Mix, "Mix");
			gainOut.init(PID::GainOut, "Gain Out");
			makeParameter(scListen, PID::SCListen, Button::Type::kToggle, "Listen");
			makeParameter(unityGain, PID::UnityGain, Button::Type::kToggle, "Unity");
			makeParameter(power, PID::Power);
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			{
				auto b = layout(0, 0, 3, 1);
				auto w = b.getWidth();
				auto h = b.getHeight() / 6.f;
				auto x = b.getX();
				auto y = b.getY();
				macro.setBounds(BoundsF(x, y, w, h).toNearestInt());
				y += h;
				scGain.setBounds(BoundsF(x, y, w, h).toNearestInt());
				y += h;
				scAuto.setBounds(BoundsF(x, y, w, h).toNearestInt());
				y += h;
				scListen.setBounds(BoundsF(x, y, w, h).toNearestInt());
				y += h;
				gainIn.setBounds(BoundsF(x, y, w, h).toNearestInt());
				y += h;
				unityGain.setBounds(BoundsF(x, y, w, h).toNearestInt());
			}
			layout.place(gainWet, 0, 1, 1, 1);
			layout.place(mix, 1, 1, 1, 1);
			layout.place(gainOut, 2, 1, 1, 1);
			layout.place(power, 0, 2, 3, 1);
			layout.place(buttonColours, 0, 3, 3, 1);
			layout.place(coloursEditor, 0, 0, 3, 3);
		}

		KnobAbsorb macro;
		KnobAbsorb scGain;
		ButtonSCAutogain scAuto;
		Button scListen;
		KnobAbsorb gainIn;
		Button unityGain;
		KnobAbsorb gainWet, mix, gainOut;
		ButtonPower power;
		ColoursEditor coloursEditor;
		ButtonColours buttonColours;
	};
}