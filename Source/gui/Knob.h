#pragma once
#include "Button.h"

namespace gui
{
	struct Knob :
		public Comp
	{
        static constexpr float DragSpeed = .3f;
        static constexpr float SensitiveDrag = .15f;
        static constexpr float WheelSpeed = .02f;

		using Func = std::function<void()>;
		using OnDrag = std::function<void(const PointF&, const Mouse&)>;
		using OnMouse = std::function<void(const Mouse&)>;
		using OnPaint = std::function<void(Graphics&, Knob&)>;

        enum kCBs { kEnterExitCB, kDownUpCB, kUpdateParameterCB, kNumCallbacks };
        enum kVals { Value, ValMod, ModDepth, ModBias, NumValTypes };

        Knob(Utils&, const String& uID);

        bool hitTest(int x, int y) override
        {
            if (!radialHitbox)
                return true;
            const auto bounds = maxQuadIn(getLocalBounds());
            const auto rad = bounds.getWidth() * .5f;
			const PointF centre
			(
				bounds.getX() + rad,
				bounds.getY() + rad
			);
            const auto pt = Point(x, y).toFloat();
            const LineF line(centre, pt);
			const auto dist = line.getLength();
			return dist <= rad;
        }

        void mouseEnter(const Mouse&) override;

        void mouseExit(const Mouse&) override;

        void mouseDown(const Mouse&) override;

        void mouseDrag(const Mouse&) override;

        void mouseUp(const Mouse&) override;

        void mouseWheelMove(const Mouse&, const MouseWheel&) override;

        void mouseDoubleClick(const Mouse&) override;

        void paint(Graphics& g) override;

        std::vector<PID> pIDs;
        std::vector<float> values;
        Func onEnter, onExit, onDown, onDoubleClick;
        OnDrag onDrag;
        OnMouse onUp, onWheel;
        OnPaint onPaint;
        PointF dragXY, lastPos;
        bool hidesCursor, active, radialHitbox;
	};

    struct ModDial :
        public Knob
    {
        ModDial(Utils&, const String&);

        // pIDs, numPIDs
        void attach(PID*, int);

        // pID
        void attach(PID);

        void paint(Graphics&) override;

    private:
        std::vector<Param*> prms;
        Path path;
        bool showBias;
    public:
        bool verticalDrag;
    };

    // pIDs, knob, verticalDrag
    void makeParameters(const std::vector<PID>&, Knob&, bool = true, bool = false);

    // pID, knob, verticalDrag
    void makeParameter(PID, Knob&, bool = true, bool = false);

    // knob
    void makeKnob(Knob&);

    // knob
    void makeSlider(Knob&);

    // knob
    void makeTextKnob(Knob&);

    // pID, knob, verticalDrag, inverted
    void makeKnob(PID, Knob&, bool = true, bool = false);

    // pID, knob
    void makeSlider(PID, Knob&);

    // pID, knob
    void makeTextKnob(PID, Knob&);

    // modDial, knob
    void locateAtKnob(ModDial&, const Knob&);

    // modDial, horizontalSlider
    void locateAtSlider(ModDial&, const Knob&);
}

/*
low stepsizes reveil that it picks the wrong points for the parameter values
wanna implement linear interpolation between points
*/