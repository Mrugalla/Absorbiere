#pragma once
#include "Knob.h"

namespace gui
{
	struct KnobAbsorb :
		public Comp
	{
		KnobAbsorb(Utils& u, const String& uID) :
			Comp(u, uID),
			knob(u, uID + "k"),
			label(u, uID + "l"),
			modDial(u, uID + "m")
		{
			layout.init
			(
				{ 1 },
				{ 8, 2 }
			);

			addAndMakeVisible(knob);
			addAndMakeVisible(label);
			setInterceptsMouseClicks(false, true);
		}

		void init(PID pID, const String& txt)
		{
			makeKnob(pID, knob);
			makeTextLabel(label, txt, font::dosisMedium(), Just::centred, CID::Txt);
			const bool modulatable = utils.getParam(pID).isModulatable();
			if (!modulatable)
				return;
			addAndMakeVisible(modDial);
			modDial.attach(pID);
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			layout.place(knob, 0, 0, 1, 1);
			layout.place(label, 0, 1, 1, 1);
			locateAtKnob(modDial, knob);
		}

		Knob knob;
		Label label;
		ModDial modDial;
	};
}