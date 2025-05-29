#pragma once
#include "Button.h"

namespace gui
{
	struct DropDownMenu :
		public Comp
	{
		// u, uID
		DropDownMenu(Utils&, const String&);

		void paint(Graphics&) override;

		// onPaint, onClick, uID
		void add(Button::OnPaint, Button::OnClick, const String&);

		// onClick, text, uID, tooltip
		void add(Button::OnClick, const String&, const String&, const String&);

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