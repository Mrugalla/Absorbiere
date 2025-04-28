#pragma once
#if PPDHasTuningEditor
#include "Button.h"
#include "../audio/dsp/KeySelector.h"

namespace gui
{
	struct KeySelectorEditor :
		public Comp
	{
		using KeySelector = dsp::KeySelector;
		static constexpr int NumKeys = KeySelector::NumKeys;

		enum kAnis { kXenUpdateCB, kKeysUpdateCB, kNumCallbacks };

		// utils, keyselector, keyselectorEnabled
		KeySelectorEditor(Utils&, KeySelector&, PID);

		void resized() override;
	
		KeySelector& selector;
		std::array<Button, NumKeys> keyButtons;
		Button keysEnabled;
		int numKeys;
	private:
		const int getXen() const noexcept;

		void initKeyButtons();
	};
}

#endif