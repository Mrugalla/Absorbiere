#pragma once
#include "Layout.h"
#include "Utils.h"
#include "Cursor.h"

namespace gui
{
	struct Comp :
		public Component
	{
		static constexpr float LockAlpha = .5f;
		static constexpr float AniLengthMs = 200.f;

		// utils, uniqueID
		Comp(Utils&, const String&);

		~Comp();

		// adding and removing components
		void add(Comp& comp, bool visible = true)
		{
			addChildComponent(comp);
			comp.setVisible(visible);
			childComps.push_back(&comp);
		}

		void remove(Comp& comp)
		{
			removeChildComponent(&comp);
			childComps.erase(std::remove(childComps.begin(), childComps.end(), &comp), childComps.end());
		}

		int getNumChildren() const noexcept
		{
			return static_cast<int>(childComps.size());
		}

		const Comp& getChild(int i) const noexcept
		{
			return *childComps[i];
		}

		Comp& getChild(int i) noexcept
		{
			return *childComps[i];
		}

		Comp* getHovered(Point screenPos)
		{
			const auto numChildren = getNumChildren();
			for (auto i = numChildren - 1; i >= 0; --i)
			{
				auto& child = getChild(i);
				if (child.isShowing())
				{
					const auto screenBounds = child.getScreenBounds();
					if (screenBounds.contains(screenPos))
						return child.getHovered(screenPos);
				}
			}
			return this;
		}

		void setLocked(bool);

		// callbacks
		void add(const Callback&);

		// layout
		// xL, yL
		void initLayout(const String&, const String&);

		BoundsF getBoundsPostTransform() const noexcept
		{
			const auto b = getBounds().toFloat();
			return b.transformedBy(getTransform());
		}

		void setTranslation(float x, float y) noexcept
		{
			const auto postBounds = getBoundsPostTransform();
			const auto postX = postBounds.getX();
			if (postX < 0.f)
				x -= postX;
			const auto postY = postBounds.getY();
			if (postY < 0.f)
				y -= postY;
			const auto& editor = utils.pluginTop;
			const auto eWidth = editor.getWidth();
			const auto eHeight = editor.getHeight();
			const auto postRight = postBounds.getRight();
			if (postRight > eWidth)
				x -= (postRight - eWidth);
			const auto postBottom = postBounds.getBottom();
			if (postBottom > eHeight)
				y -= (postBottom - eHeight);

			setTransform
			(
				getTransform().withAbsoluteTranslation(x, y)
			);
		}

		void translateRelative(float x, float y) noexcept
		{
			const auto transform = getTransform();
			x += transform.getTranslationX();
			y += transform.getTranslationY();
			setTranslation(x, y);
		}

		void setScale(float x) noexcept
		{
			scale = x;
			const auto transScale = getTransScale();
			const auto transFinal = getTransFinal(transScale);
			setTransform(transFinal);
		}

		void scaleRelative(float x) noexcept
		{
			const auto transform = getTransform();
			setScale(scale + x);
		}

		void setShear(float x, float y)
		{
			shearX = x;
			shearY = y;
			const auto transform = getTransform();
			const auto transScale = getTransScale();
			const auto transFinal = getTransFinal(transScale);
			setTransform(transFinal);
		}

		void shearRelative(float x, float y) noexcept
		{
			shearX += x;
			shearY += y;
			setShear(shearX, shearY);
		}

		// events
		void addEvt(const evt::Evt&);

		void notify(const evt::Type, const void* = nullptr);

		// mouse events
		void mouseEnter(const Mouse&) override;

		void mouseUp(const Mouse&) override;

		void setTooltip(const String&);

		Utils& utils;
		Layout layout;
		String tooltip;
		std::vector<evt::Member> members;
		Callbacks callbacks;
		float scale, shearX, shearY;
	private:
		std::vector<Comp*> childComps;

		// events backend
		void deregisterCallbacks();

		void registerCallbacks();

		FineAssTransform getTransScale() const noexcept
		{
			const auto sBounds = getBounds().toFloat();
			const auto sX = sBounds.getX();
			const auto sY = sBounds.getY();
			const auto sW = sBounds.getWidth();
			const auto sH = sBounds.getHeight();
			const auto sW2 = sW * .5f;
			const auto sH2 = sH * .5f;
			const auto transOX = -sX - sW2;
			const auto transOY = -sY - sH2;
			const auto transOrigin = FineAssTransform::translation(transOX, transOY);
			return transOrigin.followedBy(FineAssTransform::scale(scale));
		}

		FineAssTransform getTransFinal(FineAssTransform originCentred)
		{
			const auto lastBounds = getBoundsPostTransform();
			const auto lX = lastBounds.getX();
			const auto lY = lastBounds.getY();
			const auto lW = lastBounds.getWidth();
			const auto lH = lastBounds.getHeight();
			const auto lW2 = lW * .5f;
			const auto lH2 = lH * .5f;
			const PointF lCentre(lX + lW2, lY + lH2);
			return originCentred
				.followedBy(FineAssTransform::shear(shearX, shearY))
				.followedBy(FineAssTransform::translation(lCentre.x, lCentre.y));
		}

		String getTransformString()
		{
			const auto transform = getTransform();
			const auto transX = transform.getTranslationX();
			const auto transY = transform.getTranslationY();
			return String(transX) + ":" +
				String(transY) + ":" +
				String(scale) + ":" +
				String(shearX) + ":" +
				String(shearY);
		}

		void setTransformFromString(const String& s)
		{
			if (s.isEmpty())
				return;
			const auto parts = StringArray::fromTokens(s, ":", "");
			const auto transX = parts[0].getFloatValue();
			const auto transY = parts[1].getFloatValue();
			setTranslation(transX, transY);
			scale = parts[2].getFloatValue();
			shearX = parts[3].getFloatValue();
			shearY = parts[4].getFloatValue();
			const auto transScale = getTransScale();
			const auto transFinal = getTransFinal(transScale);
			setTransform(transFinal);
		}
	};
}