/*
-----------------------------------------------------------------------------------------
~~~ concepts ~~~

~~~ todo ~~~

oversampling?

linear phase eq lowpass for pre-sc rm as blend alternative

sidechain
    should also handle midi and transport blocks

resonator improvements
    resonator with better autogained q
    resonatorbank with SIMD

ToThink
    -

Param db valToStr can be -inf (gain wet [-inf, 0])
    but it can also be not (gain out [-12,12])
GUI
    SharedPluginState (customizable state)
        fonts für verschiedene funktionen
            (zB knob-labels, titel, etc)
        sensitivity
            drag, wheel, key
        defaultvalue
            doubleclick, key
AUDIO
    FFT
        replace by 3rd party lib for performance (kiss fft or pffft)
    Oversampler
        make convolver FFT-based
        implement 4x
    sidechain
        envfol
            should appear to be added to macro
            sample-accurate
    MixProcessor
        code readability
        add latency compensation delay
        write cpp

-----------------------------------------------------------------------------------------

C:\Program Files\Image-Line\FL Studio 2024\FL64.exe
C:\Program Files\Steinberg\Cubase 9.5\Cubase9.5.exe

-----------------------------------------------------------------------------------------

*/