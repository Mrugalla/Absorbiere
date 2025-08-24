#pragma once
#include "TextEditor.h"

namespace gui
{
	struct ParameterEditor :
		public TextEditor
	{
		ParameterEditor(Utils&);
	private:
		std::vector<param::PID> pIDs;
	};
}