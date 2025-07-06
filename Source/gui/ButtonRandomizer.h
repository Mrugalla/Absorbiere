#pragma once
#include "Button.h"
#include "../arch/RandomSeed.h"

namespace gui
{
    struct ButtonRandomizer :
        public Button
    {
        using RandomSeed = arch::RandSeed;
        using RandFunc = arch::RandFunc;

        // utils, randomizerID
        ButtonRandomizer(Utils&, String&&);

        void add(PID);

        void add(Param*);

        void add(std::vector<Param*>&&);

        void add(const std::vector<Param*>&);

        void add(const RandFunc&);

        // seedUp, isAbsolute
        void operator()(bool, bool);

        void mouseEnter(const Mouse&) override;
    private:
        std::vector<Param*> randomizables;
        std::vector<RandFunc> randFuncs;
        RandomSeed randomizer;
    
        String makeTooltip();
    };
}