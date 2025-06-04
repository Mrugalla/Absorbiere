#include "Editor.h"

namespace gui
{
    evt::Evt makeEvt(Editor& editor)
    {
        return [&e = editor](evt::Type type, const void*)
        {
            switch (type)
            {
            case evt::Type::ThemeUpdated:
                return e.repaint();
            case evt::Type::ClickedEmpty:
                e.utils.editLayout(false);
                //e.patchBrowser.setVisible(false);
                e.parameterEditor.setActive(false);
                e.editorComp.setVisible(true);
                e.giveAwayKeyboardFocus();
                return;
            }
        };
    }

    void loadSize(Editor& editor)
    {
        const auto& user = *editor.audioProcessor.state.props.getUserSettings();
        const auto editorWidth = user.getDoubleValue("editorwidth", EditorWidth);
        const auto editorHeight = user.getDoubleValue("editorheight", EditorHeight);
        editor.setOpaque(true);
        editor.setResizable(true, true);
        editor.setSize
        (
            static_cast<int>(editorWidth),
            static_cast<int>(editorHeight)
        );
    }

	void saveSize(Editor& editor)
	{
        auto& user = *editor.audioProcessor.state.props.getUserSettings();
        const auto editorWidth = static_cast<double>(editor.getWidth());
        const auto editorHeight = static_cast<double>(editor.getHeight());
        user.setValue("editorwidth", editorWidth);
        user.setValue("editorheight", editorHeight);
	}

    bool canResize(Editor& editor)
    {
        if (editor.getWidth() < EditorMinWidth)
        {
            editor.setSize(EditorMinWidth, editor.getHeight());
            return false;
        }
        if (editor.getHeight() < EditorMinHeight)
        {
            editor.setSize(editor.getWidth(), EditorMinHeight);
            return false;
        }
        return true;
    }

    Editor::Editor(Processor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        utils(*this, p),
        layout(),
        evtMember(utils.eventSystem, makeEvt(*this)),
        tooltip(utils),
        toast(utils),
		parameterEditor(utils),
        callback([&]()
        {
        }, 0, cbFPS::k_1_875, false),
        title(utils, "title"),
        layoutEditor(utils),
        editorComp(layoutEditor)
    {
        layout.init
        (
            { 1 },
            { 1, 13, 1 }
        );
        addChildComponent(toast);
		addAndMakeVisible(title);
        addAndMakeVisible(tooltip);
		addAndMakeVisible(editorComp);
        addChildComponent(parameterEditor);
        addChildComponent(layoutEditor);

        layoutEditor.init(&editorComp);
        makeTextLabel(title, "Absorbiere, by Florian Mrugalla", font::flx(), Just::centred, CID::Txt, "");
        title.autoMaxHeight = true;
        utils.add(&callback);
        loadSize(*this);
    }

    void Editor::paint(Graphics& g)
    {
        g.fillAll(getColour(CID::Bg));
    }
    
    void Editor::paintOverChildren(Graphics&)
    {
       // layout.paint(g, getColour(CID::Hover));
    }

    void Editor::resized()
    {
		if (!canResize(*this))
			return;
        saveSize(*this);
		layoutEditor.setBounds(getLocalBounds());
        utils.resized();
        layout.resized(getLocalBounds());
        tooltip.setBounds(layout.bottom().toNearestInt());

		layout.place(title, 0, 0, 1, 1);
		layout.place(editorComp, 0, 1, 1, 1);
		layout.place(layoutEditor, 0, 1, 1, 1);

        const auto toastWidth = static_cast<int>(utils.thicc * 28.f);
        const auto toastHeight = toastWidth * 3 / 4;
        toast.setSize(toastWidth, toastHeight);
		parameterEditor.setSize(toastWidth * 3, toastHeight);
	}

    void Editor::mouseEnter(const Mouse&)
    {
        evtMember(evt::Type::TooltipUpdated);
    }

    void Editor::mouseUp(const Mouse&)
    {
        evtMember(evt::Type::ClickedEmpty);
    }
}