#pragma once
#include "PanelEffect.h"
#include "KnobAbsorb.h"
#include "../audio/dsp/AbsorbiereAxiom.h"

namespace gui
{
	class TransferFuncEditor :
		public Comp
	{
		struct Curve :
			public Comp
		{
			Curve(Utils& u, const String& uID, PID pThreshold, PID pRatio) :
				Comp(u, uID),
				thresholdKnob(u, uID + "th"),
				ratioKnob(u, uID + "r"),
				path(),
				threshold(-420.f),
				ratio(1.f),
				knee(0.f)
			{
				addAndMakeVisible(ratioKnob);
				addAndMakeVisible(thresholdKnob);

				makeKnob(pThreshold, thresholdKnob);
				makeKnob(pRatio, ratioKnob, true, true);
				thresholdKnob.onPaint = [](Graphics&, const Knob&) {};
				ratioKnob.onPaint = [](Graphics&, const Knob&) {};
			}

			void update()
			{
				const auto w = static_cast<float>(getWidth());
				const auto h = static_cast<float>(getHeight());

				const auto tMin = static_cast<float>(dsp::ThresholdMin);
				const auto tMax = static_cast<float>(dsp::ThresholdMax);
				const auto tRange = tMax - tMin;
				const auto thicc = utils.thicc;
				const auto thicc2 = thicc * 2.f;

				const auto tNorm = (threshold - tMin) / tRange;
				const auto tKnobY = h - std::max(thicc2, tNorm * h);
				const auto tHeight = h - tKnobY;
				thresholdKnob.setBounds(BoundsF
				(
					0.f,
					tKnobY,
					w,
					tHeight
				).toNearestInt());
				ratioKnob.setBounds(BoundsF
				(
					0.f,
					0.f,
					w,
					tKnobY
				).toNearestInt());

				path.clear();
				path.startNewSubPath(0.f, h);

				const auto t0 = threshold - knee;
				const auto t0Norm = (t0 - tMin) / tRange;
				const auto y0 = h - t0Norm * h;
				const auto x0 = t0Norm * w;
				path.lineTo(x0, y0);

				const auto t1 = threshold + knee;
				const auto t1Norm = (t1 - tMin) / tRange;
				const auto x1 = t1Norm * w;
				const auto wInv = 1.f / w;
				for (auto x = x0; x < x1; x += 2)
				{
					const auto xF = static_cast<float>(x);
					const auto r = xF * wInv;
					const auto db = tMin + r * tRange;
					const auto tc = dsp::TransferDownwardsComp(db, threshold, ratio, knee);
					const auto y = h - (tc - tMin) * (h / tRange);
					path.lineTo(xF, y);
				}
				const auto tc = dsp::TransferDownwardsComp(0.f, threshold, ratio, knee);
				const auto y1 = h - (tc - tMin) * (h / tRange);
				path.lineTo(w, y1);

				areaThreshold = areaRatio = path;

				areaThreshold.lineTo(w, h);
				areaThreshold.closeSubPath();
				areaRatio.lineTo(x1, h);
				areaRatio.lineTo(x1, y1);
				areaRatio.closeSubPath();

				/*
				// first solution: add a line every 2 pixels
				const auto wInv = 1.f / w;
				for (auto x = 0; x < getWidth(); x += 2)
				{
					const auto xF = static_cast<float>(x);
					const auto r = xF * wInv;
					const auto db = tMin + r * tRange;
					const auto tc = dsp::TransferDownwardsComp(db, threshold, ratio, knee);
					const auto y = h - (tc - tMin) * (h / tRange);
					path.lineTo(xF, y);
				}
				*/

				repaint();
			}

			void paint(Graphics& g) override
			{
				setCol(g, CID::Darken);
				const auto thicc = utils.thicc;
				g.fillPath(areaThreshold);
				paintGrid(g);
				setCol(g, CID::Interact);
				Stroke stroke(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);
				g.strokePath(path, stroke);
			}

			void resized() override
			{
				update();
			}

			Knob thresholdKnob, ratioKnob;
			Path path, areaThreshold, areaRatio;
			float threshold, ratio, knee;
		private:
			void paintGrid(Graphics& g)
			{
				const auto w = static_cast<float>(getWidth());
				const auto h = static_cast<float>(getHeight());

				const Gradient linesGradient(Colour(0x0), 0.f, 0.f, getColour(CID::Hover), w, 0.f, false);
				g.setGradientFill(linesGradient);
				
				const auto tMin = static_cast<float>(dsp::ThresholdMin);
				const auto tMax = static_cast<float>(dsp::ThresholdMax);
				const auto tRange = tMax - tMin;
				const auto tRangeInv = 1.f / tRange;

				const auto thicc = utils.thicc;
				for (auto t = dsp::ThresholdMin; t < dsp::ThresholdMax; t += 10)
				{
					const auto tF = static_cast<float>(t);
					const auto tR = (tF - tMin) * tRangeInv;
					const auto y = h - tR * h;
					const auto yInt = static_cast<int>(y);
					g.drawHorizontalLine(yInt, 0.f, w);
					const auto fontHeight = static_cast<int>(std::ceil(thicc * .5f));
					g.drawFittedText(String(t) + " db", 0, yInt, getWidth(), fontHeight, Just::topRight, 1);
				}
			}
		};

		struct Meter :
			public Comp
		{
			Meter(Utils& u, const String& uID) :
				Comp(u, uID),
				data(0.f, 0.f),
				pos(0, 0),
				lineThicc(1.f)
			{
				setInterceptsMouseClicks(false, false);
			}

			void update(const PointF& v)
			{
				if (data == v)
					return;
				data = v;
				const auto w = static_cast<float>(getWidth());
				const auto h = static_cast<float>(getHeight());
				const auto tMin = static_cast<float>(dsp::ThresholdMin);
				const auto tMax = static_cast<float>(dsp::ThresholdMax);
				const auto tRange = tMax - tMin;
				const auto y = h - (data.y - tMin) * (h / tRange);
				const auto x = w * (data.x - tMin) / tRange;
				const auto off = lineThicc * .5f;
				const auto xInt = static_cast<int>(x - off);
				const auto yInt = static_cast<int>(y - off);
				if (pos.x == xInt && pos.y == yInt)
					return;
				pos.setXY(xInt, yInt);
				repaint();
			}

			void paint(Graphics& g) override
			{
				g.setColour(getColour(CID::Interact));
				g.fillEllipse(static_cast<float>(pos.x), static_cast<float>(pos.y), lineThicc, lineThicc);
			}

			void resized() override
			{
				const auto thicc = utils.thicc;
				lineThicc = std::max(2.f, thicc * 5.f);
			}
		private:
			PointF data;
			Point pos;
			float lineThicc;
		};
	public:

		TransferFuncEditor(Utils& u, const String& uID,
			PID pThreshold, PID pRatio, PID pKnee, const std::atomic<PointF>& v) :
			Comp(u, uID),
			curve(u, uID + "c", pThreshold, pRatio),
			meter(u, uID + "m")
		{
			addAndMakeVisible(curve);
			addAndMakeVisible(meter);

			add(Callback([&]()
			{
				meter.update(v);

				const auto& thresholdParam = utils.getParam(pThreshold);
				const auto& ratioParam = utils.getParam(pRatio);
				const auto& kneeParam = utils.getParam(pKnee);

				const auto _threshold = thresholdParam.getValueDenorm();
				const auto _ratio = ratioParam.getValueDenorm();
				const auto _knee = kneeParam.getValueDenorm();

				if (curve.threshold != _threshold ||
					curve.ratio != _ratio ||
					curve.knee != _knee)
				{
					curve.threshold = _threshold;
					curve.ratio = _ratio;
					curve.knee = _knee;
					curve.update();
				}
			}, 0, cbFPS::k30, true));
		}

		void resized() override
		{
			const auto bounds = getLocalBounds().toFloat().reduced(utils.thicc * 5.f).toNearestInt();
			curve.setBounds(bounds);
			meter.setBounds(bounds);
		}
	private:
		Curve curve;
		Meter meter;
	};

	struct DuckCompEditor :
		public PanelEffect
	{
		DuckCompEditor(Utils& u, const String& uID,
			PID pThreshold, PID pRatio, PID pKnee, PID pBlend, PID pAtk, PID pRls, const std::atomic<PointF>& v) :
			PanelEffect(u, uID, "Comp:"),
			blend(u, uID + "b"),
			atk(u, uID + "a"),
			rls(u, uID + "r"),
			knee(u, uID + "k"),
			labelGroup(),
			transferFuncEditor(u, uID + "tfe", pThreshold, pRatio, pKnee, v)
		{
			layout.init
			(
				{ 1, 1, 1, 1, 3 },
				{ 1, 5 }
			);

			addAndMakeVisible(blend);
			addAndMakeVisible(atk);
			addAndMakeVisible(rls);
			addAndMakeVisible(knee);
			addAndMakeVisible(transferFuncEditor);

			blend.init(pBlend, "Blend");
			atk.init(pAtk, "Attack");
			rls.init(pRls, "Release");
			knee.init(pKnee, "Knee");
			labelGroup.add(blend.label);
			labelGroup.add(atk.label);
			labelGroup.add(rls.label);
			labelGroup.add(knee.label);
		}

		void resized() override
		{
			resizeLayout();
			layout.place(blend, 0, 1, 1, 1);
			layout.place(atk, 1, 1, 1, 1);
			layout.place(rls, 2, 1, 1, 1);
			layout.place(knee, 3, 1, 1, 1);
			labelGroup.setMaxHeight(utils.thicc);
			layout.place(transferFuncEditor, 4, 1, 1, 1);
		}

	private:
		KnobAbsorb blend, atk, rls, knee;
		LabelGroup labelGroup;
		TransferFuncEditor transferFuncEditor;
	};
}
// WANNA implement hold in duck compressor