#pragma once
#include "Button.h"

namespace gui
{
	struct TextEditor :
		public Button
	{
		enum { cbEmpty, cbKeyFocus, cbCaret };

		// u, emptyString
		TextEditor(Utils&, const String & = "");

		bool keyPressed(const KeyPress&) override;

		void resized() override;

		void clear();

		void setText(const String&);

		void addText(const String&);

		void paste();

		bool isEmpty() const noexcept;

		Label labelEmpty;
		String txt, txtEmpty;
		int caret, emptyAniIdx;
		std::function<void()> onEnter;
		std::function<void(const KeyPress&)> onKeyPress;
	private:
		void updateLabel();
	};
}

/*
add ctrl+c ctrl+v ctrl+x functionality
make the caret have a different colour
*/