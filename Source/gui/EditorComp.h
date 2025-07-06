#pragma once
#include "LayoutEditor.h"
#include "ManifestOfWisdom.h"
#include "ColoursEditor.h"
#include "ButtonPower.h"
#include "KnobAbsorb.h"
#include "ButtonSCAutogain.h"
#include "OscilloscopeEditor.h"

namespace gui
{
	struct EditorComp :
		public Comp
	{
		EditorComp(CompPower& compPower, LayoutEditor& layoutEditor) :
			Comp(layoutEditor.utils),
			scope(utils, utils.audioProcessor.pluginProcessor.scope),
			macro(utils),
			scGain(utils),
			scListen(utils),
			scAuto(utils),
			gainWet(utils),
			mix(utils),
			gainOut(utils),
			power(compPower),
			coloursEditor(utils),
			buttonColours(coloursEditor),
			manifest(utils),
			buttonManifest(manifest),
			labelGroup()
		{
			layout.init
			(
				{ 2, 2, 3, 3, 2, 2, 2, 2 },
				{ 13, 2, 2 }
			);

			addAndMakeVisible(scope);
			addChildComponent(manifest);
			addChildComponent(coloursEditor);
			add(macro);
			add(scGain);
			add(scAuto);
			add(scListen);
			add(gainWet);
			add(mix);
			add(gainOut);
			add(power);
			add(buttonColours);
			add(buttonManifest);

			macro.init(PID::Macro, "Macro");
			scGain.init(PID::SCGain, "SC Gain");
			gainWet.init(PID::GainWet, "Gain Wet");
			mix.init(PID::Mix, "Mix");
			gainOut.init(PID::GainOut, "Gain Out");
			makeParameter(scListen, PID::SCListen, Button::Type::kToggle, "Listen");
			makeParameter(power, PID::Power);

			labelGroup.add(macro.label);
			labelGroup.add(scGain.label);
			labelGroup.add(scListen.label);
			labelGroup.add(gainOut.label);
		}

		void paint(Graphics& g) override
		{
			const auto c0 = getColour(CID::Bg);
			const auto c1 = c0.overlaidWith(getColour(CID::Darken));
			const auto bounds = layout(0, 1, 8, 2);
			const PointF p0(bounds.getX(), bounds.getY());
			const PointF p1(bounds.getX(), bounds.getBottom());
			Gradient gradient(c1, p0, c0, p1, false);
			g.setGradientFill(gradient);
			g.fillRect(bounds);
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			const auto top = layout.top().toNearestInt();
			scope.setBounds(top);
			coloursEditor.setBounds(top);
			manifest.setBounds(top);
			layout.place(buttonColours, 0, 1, 1, 1);
			layout.place(buttonManifest, 0, 2, 1, 1);
			layout.place(macro, 1, 1, 1, 2);
			layout.place(scGain, 2, 1, 1, 2);
			layout.place(scAuto, 3, 1, 1, 2);
			layout.place(scListen, 4, 1, 1, 1);
			layout.place(power, 4, 2, 1, 1);
			layout.place(gainWet, 5, 1, 1, 2);
			layout.place(mix, 6, 1, 1, 2);
			layout.place(gainOut, 7, 1, 1, 2);
			labelGroup.setMaxHeight(utils.thicc);
		}
	private:
		OscilloscopeEditor scope;
		KnobAbsorb macro;
		KnobAbsorb scGain;
		ButtonSCAutogain scAuto;
		Button scListen;
		KnobAbsorb gainWet, mix, gainOut;
		ButtonPower power;
		ColoursEditor coloursEditor;
		ButtonColours buttonColours;
		ManifestOfWisdom manifest;
		ButtonWisdom buttonManifest;
		LabelGroup labelGroup;
	};
}