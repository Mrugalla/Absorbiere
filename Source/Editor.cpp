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
				//e.coloursEditor.setVisible(false);
                //e.manifestOfWisdom.setVisible(false);
                //e.patchBrowser.setVisible(false);
                //e.buttonColours.value = 0.f;
				//e.manifestOfWisdomButton.value = 0.f;
                //e.buttonColours.repaint();
				//e.manifestOfWisdomButton.repaint();
                //e.parameterEditor.setActive(false);
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
            const auto& modeParam = utils.audioProcessor.params(PID::Mode);
            const auto val = static_cast<int>(std::round(modeParam.getValModDenorm()));
            duckEditor.setVisible(val == 0);
        }, 0, cbFPS::k7_5, true),
        title(utils),
		mode(utils),
        duckEditor(utils),
		ioEditor(utils)
    {
        layout.init
        (
            { 5, 3 },
            { 1, 1, 21, 1 }
        );
        addChildComponent(toast);
		addAndMakeVisible(title);
		addAndMakeVisible(mode);
		addAndMakeVisible(duckEditor);
		addAndMakeVisible(ioEditor);
        addChildComponent(parameterEditor);

        makeTextLabel(title, "Absorbiere", font::flx(), Just::centred, CID::Txt, "");
        title.autoMaxHeight = true;
        mode.attach(PID::Mode);
        {
            const auto& modeParam = utils.audioProcessor.params(PID::Mode);
            const auto val = static_cast<int>(std::round(modeParam.getValModDenorm()));
            duckEditor.setVisible(val == 0);
        }

		addAndMakeVisible(tooltip);
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
        utils.resized();
        layout.resized(getLocalBounds());
        tooltip.setBounds(layout.bottom().toNearestInt());

		layout.place(title, 0, 0, 1, 1);
		layout.place(mode, 0, 1, 1, 1);
		layout.place(duckEditor, 0, 2, 1, 1);
		layout.place(ioEditor, 1, 0, 1, 3);

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

// WANNA implement hold in duck compressor