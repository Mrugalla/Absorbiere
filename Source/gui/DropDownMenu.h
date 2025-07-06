#pragma once
#include "Button.h"

namespace gui
{
	struct DropDownMenu :
		public Comp
	{
		// u
		DropDownMenu(Utils&);

		void paint(Graphics&) override;

		// onPaint, onClick
		void add(Button::OnPaint, Button::OnClick);

		// onClick, text, tooltip
		void add(Button::OnClick, const String&, const String&);

		void init();

		void resized() override;
	private:
		std::vector<std::unique_ptr<Button>> buttons;
		LabelGroup labelGroup;
	};

	struct ButtonDropDown :
		public Button
	{
		ButtonDropDown(Utils&);

		// dropdown, title, tooltip
		void init(DropDownMenu&, const String&, const String&);
	};
}