#pragma once
#include "Knob.h"

namespace gui
{
#if PPDHasLayoutEditor
	struct LayoutEditor :
		public Comp
	{
		LayoutEditor(Utils& u) :
			Comp(u, "layouteditor"),
			editor(nullptr),
			hovered(nullptr),
			selected(nullptr)
		{
			setOpaque(false);

			addEvt([&](evt::Type type, const void*)
			{
				switch (type)
				{
				case evt::Type::ClickedEmpty:
					u.editLayout(false);
					setVisible(false);
					return;
				}
			});
		}

		void init(Comp* _editor)
		{
			editor = _editor;
			add(Callback([&]()
			{
				setVisible(utils.isEditingLayout());
			}, 0, cbFPS::k_1_875, true));
		}

		void paint(Graphics& g) override
		{
			if (hovered)
				paint(g, hovered, CID::Txt);
			if (selected)
				paint(g, selected, CID::Interact);
		}

		void mouseDown(const Mouse& mouse) override
		{
			dragXY = mouse.position;
			if(select(mouse))
				repaint();
		}

		void mouseMove(const Mouse& mouse) override
		{
			const auto nComp = getComp(mouse);
			if (hovered == nComp)
				return;
			hovered = nComp;
			repaint();
		}

		void mouseDrag(const Mouse& mouse) override
		{
			if (!selected)
				return;
			const auto nPos = mouse.position;
			const auto delta = nPos - dragXY;
			const auto speed = mouse.mods.isShiftDown() ? Knob::SensitiveDrag : 1.f;
			auto nXY = delta * speed;
			if (mouse.mods.isAltDown())
			{
				nXY /= Tau * 2.f;
				selected->shearRelative(nXY.x, nXY.y);
			}
			else
			{
				const auto& top = utils.pluginTop;
				const auto topScreen = top.getScreenBounds().toFloat();
				const auto minDimen = std::min(topScreen.getWidth(), topScreen.getHeight());
				nXY /= minDimen;
				selected->translateRelative(nXY.x, nXY.y);
			}
			dragXY = nPos;
		}

		void mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel) override
		{
			if (!selected)
				return;
			const auto speed = mouse.mods.isShiftDown() ? Knob::SensitiveDrag : .5f;
			const auto delta = wheel.deltaY * speed;
			selected->scaleRelative(delta);
		}

		void mouseDoubleClick(const Mouse& mouse) override
		{
			if (mouse.mouseWasDraggedSinceMouseDown())
				return;
			if (mouse.mods.isAltDown())
			{
				if (selected)
					selected->setShear(0.f, 0.f);
			}
			else
			{
				selected->setScale(1.f);
			}
		}

		void mouseUp(const Mouse&) override
		{
			// this is important lol
		}
	private:
		Comp *editor, *hovered, *selected;
		PointF dragXY;

		void paint(Graphics& g, Comp* comp, CID cID)
		{
			const auto screenBounds = getScreenBounds();
			const auto screenBoundsComp = comp->getScreenBounds();
			const auto boundsComp = screenBoundsComp
				.withX(screenBoundsComp.getX() - screenBounds.getX())
				.withY(screenBoundsComp.getY() - screenBounds.getY());
			g.setColour(getColour(cID).withRotatedHue(.333f));
			const auto thicc = utils.thicc;
			g.drawRect(boundsComp.toFloat().reduced(thicc), thicc);
		}

		bool select(const Mouse& mouse) noexcept
		{
			const auto nComp = getComp(mouse);
			if (selected == nComp)
				return false;
			selected = nComp;
			return true;
		}

		Comp* getComp(const Mouse& mouse) const noexcept
		{
			const auto screenPos = mouse.getScreenPosition();
			return editor->getHovered(screenPos);
		}
	};
#else
	struct LayoutEditor :
		public Comp
	{
		LayoutEditor(Utils& u) :
			Comp(u)
		{
			setInterceptsMouseClicks(false, false);
		}

		void init(Comp*) {}
	};
#endif
	struct ButtonLayoutEditor :
		public Button
	{
		ButtonLayoutEditor(LayoutEditor& editor) :
			Button(editor.utils)
		{
			const auto symbol = juce::CharPointer_UTF8("\xf0\x9f\xa7\xb1");
			makeTextButton(*this, String(symbol), "Click here to edit the layout of the interface's components.", CID::Interact);
			label.autoMaxHeight = true;
			type = Button::Type::kToggle;
			value = utils.isEditingLayout() ? 1.f : 0.f;

			onClick = [&](const Mouse&)
			{
				utils.switchEditLayout();
				const auto isEditing = utils.isEditingLayout();
				value = isEditing ? 1.f : 0.f;
				editor.setVisible(isEditing);
			};
		}
	};
}