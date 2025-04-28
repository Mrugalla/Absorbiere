#pragma once
#include "gui/Tooltip.h"
#include "gui/Knob.h"

namespace gui
{
	using AudioProcessorEditor = juce::AudioProcessorEditor;

    struct Editor :
        public AudioProcessorEditor
    {
        Editor(Processor&);

		void paint(Graphics&) override;
        void paintOverChildren(Graphics&) override;
        void resized() override;
        void mouseEnter(const Mouse&) override;
        void mouseUp(const Mouse&) override;

        Processor& audioProcessor;
        Utils utils;
        Layout layout;
        evt::Member evtMember;
        
        Tooltip tooltip;

        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    };
}