#pragma once
#include "Knob.h"
#include "ButtonRandomizer.h"
#include "../audio/dsp/EnvelopeFollower.h"

namespace gui
{
	class EnvelopeFollowerEditor :
		public Comp
	{
		using EnvFol = dsp::EnvelopeFollower;

		struct Visualizer :
			public Comp
		{
			// u, uID, envFol
			Visualizer(Utils&, const String&, const EnvFol&);

			void resized() override;

			void paint(Graphics& g) override;
		private:
			Image img;
			float y0;
		};

	public:
		// u, uID, envFol, gain, attack, decay, smooth
		EnvelopeFollowerEditor(Utils&, const String&,
			const EnvFol&, PID, PID, PID, PID);

		void paint(Graphics&) override;

		void resized() override;

	private:
		Visualizer visualizer;
		Label title, gainLabel, attackLabel, decayLabel, smoothLabel;
		Knob gain, attack, decay, smooth;
		ModDial gainMod, attackMod, decayMod, smoothMod;
		ButtonRandomizer randomizer;
		LabelGroup labelGroup;
	};
}