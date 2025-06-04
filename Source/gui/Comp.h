#pragma once
#include "Layout.h"
#include "Utils.h"
#include "Cursor.h"

namespace gui
{
	struct Comp :
		public Component
	{
		struct TransformInfo
		{
			TransformInfo(Comp& _c) :
				transX(0.f),
				transY(0.f),
				scale(1.f),
				shearX(0.f),
				shearY(0.f),
				c(_c)
			{}

			void updateTranslation() noexcept
			{
				const auto& top = c.utils.pluginTop;
				const auto topScreen = top.getScreenBounds().toFloat();
				const auto minDimen = std::min(topScreen.getWidth(), topScreen.getHeight());
				const auto x = transX * minDimen;
				const auto y = transY * minDimen;
				c.setTransform
				(
					c.getTransform().withAbsoluteTranslation(x, y)
				);
				/*
				const auto postBounds = getBoundsPostTransform();
				
				const auto w = postBounds.getWidth();
				const auto h = postBounds.getHeight();
				const auto postX = postBounds.getX();
				const auto eWidth = editor.getWidth();
				const auto eHeight = editor.getHeight();
				if (postX < -w)
					x -= postX;
				const auto postY = postBounds.getY();
				if (postY < -h)
					y -= postY;
				const auto postRight = postBounds.getRight();
				if (postRight > eWidth + w)
					x -= (postRight - eWidth);
				const auto postBottom = postBounds.getBottom();
				if (postBottom > eHeight + h)
					y -= (postBottom - eHeight);
				transX = x;
				transY = y;

				c.setTransform
				(
					c.getTransform().withAbsoluteTranslation(transX, transY)
				);
				*/
			}

			void translateRelative(float x, float y) noexcept
			{
				transX += x;
				transY += y;
				updateTranslation();
			}

			void setScale(float x) noexcept
			{
				scale = x;
				const auto transScale = getTransScale();
				const auto transFinal = getTransFinal(transScale);
				c.setTransform(transFinal);
			}

			void scaleRelative(float x) noexcept
			{
				const auto transform = c.getTransform();
				setScale(scale + x);
			}

			void setShear(float x, float y)
			{
				shearX = x;
				shearY = y;
				const auto transform = c.getTransform();
				const auto transScale = getTransScale();
				const auto transFinal = getTransFinal(transScale);
				c.setTransform(transFinal);
			}

			void shearRelative(float x, float y) noexcept
			{
				shearX += x;
				shearY += y;
				setShear(shearX, shearY);
			}

			String getTransformString()
			{
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
				transX = parts[0].getFloatValue();
				transY = parts[1].getFloatValue();
				scale = parts[2].getFloatValue();
				shearX = parts[3].getFloatValue();
				shearY = parts[4].getFloatValue();
				updateTranslation();
				const auto transScale = getTransScale();
				const auto transFinal = getTransFinal(transScale);
				c.setTransform(transFinal);
			}

			float transX, transY, scale, shearX, shearY;
		private:
			Comp& c;

			BoundsF getBoundsPostTransform() const noexcept
			{
				const auto b = c.getBounds().toFloat();
				return b.transformedBy(c.getTransform());
			}

			FineAssTransform getTransScale() const noexcept
			{
				const auto sBounds = c.getBounds().toFloat();
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
		};

		static constexpr float LockAlpha = .5f;
		static constexpr float AniLengthMs = 200.f;

		// utils, uniqueID
		Comp(Utils&, const String& = "");

		~Comp();

		void resized() override
		{
			transformInfo.updateTranslation();
			layout.resized(getLocalBounds());
		}

		// adding and removing components
		// child, visible
		void add(Comp&, bool = true);

		void remove(Comp&);

		int getNumChildren() const noexcept;

		const Comp& getChild(int) const noexcept;

		Comp& getChild(int) noexcept;

		Comp* getHovered(Point);

		void setLocked(bool);

		// callbacks
		void add(const Callback&);

		// layout
		// xL, yL
		void initLayout(const String&, const String&);

		void translateRelative(float x, float y) noexcept
		{
			transformInfo.translateRelative(x, y);
		}

		void setScale(float x) noexcept
		{
			transformInfo.setScale(x);
		}

		void scaleRelative(float x) noexcept
		{
			transformInfo.scaleRelative(x);
		}

		void setShear(float x, float y)
		{
			transformInfo.setShear(x, y);
		}

		void shearRelative(float x, float y) noexcept
		{
			transformInfo.shearRelative(x, y);
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
		TransformInfo transformInfo;
	private:
		std::vector<Comp*> childComps;

		// events backend
		void deregisterCallbacks();

		void registerCallbacks();
	};
}