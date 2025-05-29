#pragma once
#include "gui/Tooltip.h"
#include "gui/ParameterEditor.h"
#include "gui/EditorComp.h"

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
        Toast toast;
        ParameterEditor parameterEditor;
        Callback callback;
        Label title;
        ManifestOfWisdom manifestOfWisdom;
		ColoursEditor coloursEditor;
        LayoutEditor layoutEditor;
		EditorComp editorComp;
		

        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    };
}