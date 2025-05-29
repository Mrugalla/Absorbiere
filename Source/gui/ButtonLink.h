#pragma once
#include "Button.h"

namespace gui
{
	struct ButtonLink :
		public Button
	{
		struct Link { String name, uID; URL url; };
		using Links = std::vector<Link>;

		// u
		ButtonLink(Utils&);

		// name, uID, url
		void init(const String&, const String&, const URL&);
	};
}