#pragma once
#include "Knob.h"
#include "../audio/dsp/Randomizer.h"
#include "ButtonRandomizer.h"

namespace gui
{
	struct RandomizerEditor :
		public Comp
	{
		using RandMod = dsp::Randomizer;

		struct Visualizer :
			public Comp
		{
			// u, uID, randMod
			Visualizer(Utils&, const String&, const RandMod&);

			void resized() override;

			void paint(Graphics&);
		private:
			Image img;
			float y0;
		};

		// u, uID, randMod, rateSync, smooth, complex, dropout
		RandomizerEditor(Utils&, const String&,
			const RandMod&, PID, PID, PID, PID);

		void paint(Graphics&);

		void resized() override;
	private:
		Visualizer visualizer;
		Label title, rateSyncLabel, smoothLabel, complexLabel, dropoutLabel;
		Knob rateSync, smooth, complex, dropout;
		ModDial rateSyncMod, smoothMod, complexMod, dropoutMod;
		ButtonRandomizer randomizer;
		LabelGroup labelGroup;
	};
}