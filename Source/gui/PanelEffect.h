#pragma once
#include "Label.h"

namespace gui
{
	struct PanelEffect :
		public Comp
	{
		PanelEffect(Utils& u, const String& name) :
			Comp(u),
			title(u)
		{
			addAndMakeVisible(title);
			makeTextLabel(title, name, font::dosisExtraBold(), Just::centred, CID::Txt);
			title.autoMaxHeight = true;
		}

		void resizeLayout()
		{
			layout.resized(getLocalBounds());
			layout.place(title, 0, 0, 1, 1);
		}

		void paint(Graphics& g) override
		{
			setCol(g, CID::Darken);
			const auto thicc = utils.thicc;
			const auto thicc3 = thicc * 3.f;
			const auto bounds = getLocalBounds().toFloat().reduced(thicc3);
			g.fillRoundedRectangle(bounds, thicc3);
		}

	private:
		Label title;
	};
}