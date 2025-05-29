#pragma once
#include "LayoutEditor.h"
#include "ManifestOfWisdom.h"
#include "ColoursEditor.h"
#include "ButtonPower.h"
#include "KnobAbsorb.h"
#include "ButtonSCAutogain.h"
#include "Oscilloscope.h"


namespace gui
{
	struct EditorComp :
		public Comp
	{
		EditorComp(LayoutEditor& layoutEditor) :
			Comp(layoutEditor.utils, "editorcomp"),
			scope(utils, "scope", utils.audioProcessor.pluginProcessor.scope),
			macro(utils, "macro"),
			scGain(utils, "scgain"),
			scListen(utils, "listen"),
			scAuto(utils),
			gainOut(utils, "gainout"),
			power(utils),
			buttonLayout(layoutEditor),
			coloursEditor(utils),
			buttonColours(coloursEditor),
			manifest(utils),
			buttonManifest(manifest),
			labelGroup()
		{
			layout.init
			(
				{ 2, 2, 2, 2, 3, 3, 2, 2, 2 },
				{ 21, 1, 3, 1 }
			);

			add(scope);
			add(macro);
			add(scGain);
			add(scAuto);
			add(scListen);
			add(gainOut);
			add(power);
			add(buttonLayout);
			add(buttonColours);
			add(buttonManifest);
			utils.getProps().getFile().revealToUser();

			macro.init(PID::Macro, "Macro");
			scGain.init(PID::SCGain, "SC Gain");
			gainOut.init(PID::GainOut, "Gain Out");
			makeParameter(scListen, PID::SCListen, Button::Type::kToggle, "Listen");
			makeParameter(power, PID::Power);

			labelGroup.add(macro.label);
			labelGroup.add(scGain.label);
			labelGroup.add(scListen.label);
			labelGroup.add(gainOut.label);
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			scope.setBounds(layout.top().toNearestInt());
			layout.place(buttonLayout, 0, 2, 1, 1);
			layout.place(buttonColours, 1, 2, 1, 1);
			layout.place(buttonManifest, 2, 2, 1, 1);
			layout.place(macro, 3, 2, 1, 1);
			layout.place(scGain, 4, 1, 1, 2);
			layout.place(scAuto, 5, 1, 1, 2);
			layout.place(scListen, 6, 2, 1, 1);
			layout.place(gainOut, 7, 2, 1, 1);
			layout.place(power, 8, 2, 1, 1);
			labelGroup.setMaxHeight(utils.thicc);
		}

	private:
		OscilloscopeEditor scope;
		KnobAbsorb macro;
		KnobAbsorb scGain;
		ButtonSCAutogain scAuto;
		Button scListen;
		KnobAbsorb gainOut;
		ButtonPower power;
		ButtonLayoutEditor buttonLayout;
		ColoursEditor coloursEditor;
		ButtonColours buttonColours;
		ManifestOfWisdom manifest;
		ButtonWisdom buttonManifest;
		LabelGroup labelGroup;
	};
}