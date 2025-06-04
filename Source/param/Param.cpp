#include "Param.h"
#include "../arch/FormulaParser.h"
#include "../arch/Range.h"
#include "../arch/Math.h"
#include "../audio/dsp/AbsorbiereAxiom.h"

namespace param
{
	String toID(const String& name)
	{
		return name.removeCharacters(" ").toLowerCase();
	}

	PID ll(PID pID, int off) noexcept
	{
		auto i = static_cast<int>(pID);
		i += (NumLowLevelParams - 1) * off;
		return static_cast<PID>(i);
	}

	PID offset(PID pID, int off) noexcept
	{
		return static_cast<PID>(static_cast<int>(pID) + off);
	}

	String toString(PID pID)
	{
		switch (pID)
		{
		case PID::Macro: return "Macro";
#if PPDHasSidechain
		case PID::SCGain: return "SC Gain";
		case PID::SCListen: return "SC Listen";
#endif
#if PPDIsNonlinear
		case PID::GainIn: return "Gain In";
		case PID::UnityGain: return "Unity Gain";
#endif
#if PPDIO == PPDIODryWet
		case PID::GainDry: return "Gain Dry";
		case PID::GainWet: return "Gain Wet";
#elif PPDIO == PPDIOWetMix
		case PID::GainWet: return "Gain Wet";
		case PID::Mix: return "Mix";
#if PPDHasDelta
		case PID::Delta: return "Delta";
#endif
#endif
		case PID::GainOut: return "Gain Out";
#if PPDHasStereoConfig
		case PID::StereoConfig: return "Stereo Config";
#endif
#if PPDHasHQ
		case PID::HQ: return "HQ";
#endif
#if PPDHasLookahead
		case PID::Lookahead: return "Lookahead";
#endif
#if PPDHasTuningEditor
			// TUNING PARAM:
		case PID::Xen: return "Xen";
		case PID::XenSnap: return "Xen Snap";
		case PID::MasterTune: return "Master Tune";
		case PID::AnchorPitch: return "Anchor Pitch";
		case PID::PitchbendRange: return "Pitchbend Range";
		case PID::TempC: return "Temp C";
		case PID::TempDb: return "Temp Db";
		case PID::TempD: return "Temp D";
		case PID::TempEb: return "Temp Eb";
		case PID::TempE: return "Temp E";
		case PID::TempF: return "Temp F";
		case PID::TempGb: return "Temp Gb";
		case PID::TempG: return "Temp G";
		case PID::TempAb: return "Temp Ab";
		case PID::TempA: return "Temp A";
		case PID::TempBb: return "Temp Bb";
		case PID::TempB: return "Temp B";
		case PID::TempC2: return "Temp C2";
		case PID::TempDb2: return "Temp Db2";
		case PID::TempD2: return "Temp D2";
		case PID::TempEb2: return "Temp Eb2";
		case PID::TempE2: return "Temp E2";
		case PID::TempF2: return "Temp F2";
		case PID::TempGb2: return "Temp Gb2";
		case PID::TempG2: return "Temp G2";
		case PID::TempAb2: return "Temp Ab2";
		case PID::TempA2: return "Temp A2";
		case PID::TempBb2: return "Temp Bb2";
		case PID::TempB2: return "Temp B2";
		case PID::TempC3: return "Temp C3";
		case PID::TempDb3: return "Temp Db3";
		case PID::TempD3: return "Temp D3";
		case PID::TempEb3: return "Temp Eb3";
		case PID::TempE3: return "Temp E3";
		case PID::TempF3: return "Temp F3";
		case PID::TempGb3: return "Temp Gb3";
		case PID::TempG3: return "Temp G3";
		case PID::TempAb3: return "Temp Ab3";
		case PID::TempA3: return "Temp A3";
		case PID::TempBb3: return "Temp Bb3";
		case PID::TempB3: return "Temp B3";
		case PID::TempC4: return "Temp C4";
		case PID::TempDb4: return "Temp Db4";
		case PID::TempD4: return "Temp D4";
		case PID::TempEb4: return "Temp Eb4";
		case PID::TempE4: return "Temp E4";
		case PID::TempF4: return "Temp F4";
		case PID::TempGb4: return "Temp Gb4";
		case PID::TempG4: return "Temp G4";
		case PID::TempAb4: return "Temp Ab4";
		case PID::TempA4: return "Temp A4";
		case PID::TempBb4: return "Temp Bb4";
		case PID::TempB4: return "Temp B4";
#endif
		case PID::Power: return "Power";

		// LOW LEVEL PARAMS:

		default: return "Invalid Parameter Name";
		}
	}

	PID toPID(const String& id)
	{
		const auto nID = toID(id);
		for (auto i = 0; i < NumParams; ++i)
		{
			auto pID = static_cast<PID>(i);
			if (nID == toID(toString(pID)))
				return pID;
		}
		return PID::NumParams;
	}

	void toPIDs(std::vector<PID>& pIDs, const String& text, const String& seperator)
	{
		auto tokens = juce::StringArray::fromTokens(text, seperator, "");
		for (auto& token : tokens)
		{
			auto pID = toPID(token);
			if (pID != PID::NumParams)
				pIDs.push_back(pID);
		}
	}

	String toTooltip(PID pID)
	{
		switch (pID)
		{
		case PID::Macro: return "Dial in the desired amount of macro modulation depth.";
#if PPDHasSidechain
		case PID::SCGain: return "Apply gain to the sidechain signal.";
		case PID::SCListen: return "Listen to the sidechain signal.";
#endif
#if PPDIsNonlinear
		case PID::GainIn: return "Apply gain to the input signal.";
		case PID::UnityGain: return "Compensates for the added input gain.";
#endif
#if PPDIO == PPDIODryWet
		case PID::GainDry: return "Apply gain to the dry signal.";
		case PID::GainWet: return "Apply gain to the wet signal.";
#elif PPDIO == PPDIOWetMix
		case PID::GainWet: return "Apply gain to the wet signal.";
		case PID::Mix: return "Mix the dry with the wet signal.";
#if PPDHasDelta
		case PID::Delta: return "Listen to the difference between the dry and the wet signal.";
#endif
#endif
		case PID::GainOut: return "Apply gain to the output signal.";
#if PPDHasStereoConfig
		case PID::StereoConfig: return "Switch between L/R and M/S mode.";
#endif
#if PPDHasHQ
		case PID::HQ: return "Apply oversampling to the signal.";
#endif

#if PPDHasLookahead
		case PID::Lookahead: return "Dis/Enabled lookahead.";
#endif
#if PPDHasTuningEditor
		// TUNING PARAMS:
		case PID::Xen: return "The xenharmony describes how many pitches per octave exist. Higher edo = smaller intervals.";
		case PID::XenSnap: return "If disabled you can explore xen scales that have less or no octaves.";
		case PID::MasterTune: return "This is the frequency the anchor pitch refers to. (Chamber Frequency)";
		case PID::AnchorPitch: return "The anchor pitch refers to the same frequency regardless of xen scale.";
		case PID::PitchbendRange: return "The pitchbend range in semitones describes how many pitches you can bend.";
		case PID::TempC: return "The temperament of the pitch class C";
		case PID::TempDb: return "The temperament of the pitch class C#";
		case PID::TempD: return "The temperament of the pitch class D";
		case PID::TempEb: return "The temperament of the pitch class D#";
		case PID::TempE: return "The temperament of the pitch class E";
		case PID::TempF: return "The temperament of the pitch class F";
		case PID::TempGb: return "The temperament of the pitch class F#";
		case PID::TempG: return "The temperament of the pitch class G";
		case PID::TempAb: return "The temperament of the pitch class G#";
		case PID::TempA: return "The temperament of the pitch class A";
		case PID::TempBb: return "The temperament of the pitch class A#";
		case PID::TempB: return "The temperament of the pitch class B";
		case PID::TempC2: return "The temperament of the pitch class C2";
		case PID::TempDb2: return "The temperament of the pitch class C#2";
		case PID::TempD2: return "The temperament of the pitch class D2";
		case PID::TempEb2: return "The temperament of the pitch class D#2";
		case PID::TempE2: return "The temperament of the pitch class E2";
		case PID::TempF2: return "The temperament of the pitch class F2";
		case PID::TempGb2: return "The temperament of the pitch class F#2";
		case PID::TempG2: return "The temperament of the pitch class G2";
		case PID::TempAb2: return "The temperament of the pitch class G#2";
		case PID::TempA2: return "The temperament of the pitch class A2";
		case PID::TempBb2: return "The temperament of the pitch class A#2";
		case PID::TempB2: return "The temperament of the pitch class B2";
		case PID::TempC3: return "The temperament of the pitch class C3";
		case PID::TempDb3: return "The temperament of the pitch class C#3";
		case PID::TempD3: return "The temperament of the pitch class D3";
		case PID::TempEb3: return "The temperament of the pitch class D#3";
		case PID::TempE3: return "The temperament of the pitch class E3";
		case PID::TempF3: return "The temperament of the pitch class F3";
		case PID::TempGb3: return "The temperament of the pitch class F#3";
		case PID::TempG3: return "The temperament of the pitch class G3";
		case PID::TempAb3: return "The temperament of the pitch class G#3";
		case PID::TempA3: return "The temperament of the pitch class A3";
		case PID::TempBb3: return "The temperament of the pitch class A#3";
		case PID::TempB3: return "The temperament of the pitch class B3";
		case PID::TempC4: return "The temperament of the pitch class C4";
		case PID::TempDb4: return "The temperament of the pitch class C#4";
		case PID::TempD4: return "The temperament of the pitch class D4";
		case PID::TempEb4: return "The temperament of the pitch class D#4";
		case PID::TempE4: return "The temperament of the pitch class E4";
		case PID::TempF4: return "The temperament of the pitch class F4";
		case PID::TempGb4: return "The temperament of the pitch class F#4";
		case PID::TempG4: return "The temperament of the pitch class G4";
		case PID::TempAb4: return "The temperament of the pitch class G#4";
		case PID::TempA4: return "The temperament of the pitch class A4";
		case PID::TempBb4: return "The temperament of the pitch class A#4";
		case PID::TempB4: return "The temperament of the pitch class B4";
#endif
		case PID::Power: return "Dis/Enable the plugin.";

		// LOW LEVEL PARAMS:

		default: return "Invalid Tooltip.";
		}
	}

	String toString(Unit pID)
	{
		switch (pID)
		{
		case Unit::Power: return "";
		case Unit::Solo: return "S";
		case Unit::Mute: return "M";
		case Unit::Percent: return "%";
		case Unit::Hz: return "hz";
		case Unit::Beats: return "";
		case Unit::Degree: return CharPtr("\xc2\xb0");
		case Unit::Octaves: return "oct";
		case Unit::Semi: return "semi";
		case Unit::Fine: return "ct";
		case Unit::Ms: return "ms";
		case Unit::Decibel: return "db";
		case Unit::Ratio: return "ratio";
		case Unit::Polarity: return CharPtr("\xc2\xb0");
		case Unit::StereoConfig: return "";
		case Unit::Voices: return "v";
		case Unit::Pan: return "%";
		case Unit::Xen: return "notes/oct";
		case Unit::Note: return "";
		case Unit::Pitch: return "";
		case Unit::Q: return "q";
		case Unit::Slope: return "db/oct";
		case Unit::Legato: return "";
		case Unit::Custom: return "";
		case Unit::FilterType: return "";
		default: return "";
		}
	}

	// PARAM::CB:

	float Param::CB::denorm() const noexcept
	{
		return param.range.convertFrom0to1(norm);
	}

	bool Param::CB::getBool() const noexcept
	{
		return norm > .5f;
	}

	// PARAM:

	Param::Mod::Mod() :
		depth(0.f),
		bias(.5f)
	{}

	Param::Param(const PID pID, const Range& _range, const float _valDenormDefault,
		const ValToStrFunc& _valToStr, const StrToValFunc& _strToVal, const Unit _unit, bool _modulatable) :
		AudioProcessorParameter(),
		id(pID),
		range(_range),
		callback([](CB) {}),
		valDenormDefault(range.snapToLegalValue(_valDenormDefault)),
		valInternal(range.convertTo0to1(valDenormDefault)),
		curValMod(-1.f),
		mod(),
		valNorm(valInternal), valMod(valNorm.load()),
		valToStr(_valToStr), strToVal(_strToVal), unit(_unit),
		locked(false), inGesture(false), modDepthAbsolute(false),
		modulatable(_modulatable)
	{
	}

	void Param::prepare() noexcept
	{
		curValMod = -1.f;
	}

	void Param::savePatch(State& state) const
	{
		const String idStr("params/" + toID(toString(id)));

		const auto v = range.convertFrom0to1(getValue());
		state.set(idStr + "/value", v);
		const auto md = getModDepth();
		state.set(idStr + "/md", md);
		const auto mb = getModBias();
		state.set(idStr + "/mb", mb);
	}

	void Param::loadPatch(const State& state)
	{
		if (isLocked())
			return;
		const String idStr("params/" + toID(toString(id)));
		auto var = state.get(idStr + "/value");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			const auto legalVal = range.snapToLegalValue(val);
			const auto valD = range.convertTo0to1(legalVal);
			setValueNotifyingHost(valD);
		}
		var = state.get(idStr + "/md");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			setModDepth(val);
		}
		var = state.get(idStr + "/mb");
		if (var)
		{
			const auto val = static_cast<float>(*var);
			setModBias(val);
		}
	}

	// called by host, normalized, thread-safe
	float Param::getValue() const
	{
		return valNorm.load();
	}

	float Param::getValueDenorm() const noexcept
	{
		return range.convertFrom0to1(getValue());
	}

	void Param::setValue(float normalized)
	{
		if (isLocked())
			return;

		if (!modDepthAbsolute || !modulatable)
			return valNorm.store(normalized);

		const auto pLast = valNorm.load();
		const auto pCur = normalized;

		const auto dLast = getModDepth();
		const auto dCur = dLast - pCur + pLast;
		setModDepth(dCur);

		valNorm.store(pCur);
	}

	void Param::setValueFromEditor(float x) noexcept
	{
		const auto lckd = isLocked();
		setLocked(false);
		setValueNotifyingHost(x);
		setLocked(lckd);
	}

	// called by editor
	bool Param::isInGesture() const noexcept
	{
		return inGesture.load();
	}

	// called by editor
	void Param::setValueWithGesture(float norm)
	{
		if (isInGesture())
			return;
		beginGesture();
		setValueFromEditor(norm);
		endGesture();
	}

	void Param::beginGesture()
	{
		inGesture.store(true);
		beginChangeGesture();
	}

	void Param::endGesture()
	{
		inGesture.store(false);
		endChangeGesture();
	}

	float Param::getModDepth() const noexcept
	{
		return mod.depth.load();
	}

	void Param::setModDepth(float v) noexcept
	{
		if (isLocked() || !modulatable)
			return;

		mod.depth.store(juce::jlimit(-1.f, 1.f, v));
	}

	float Param::calcValModOf(float modSrc) const noexcept
	{
		const auto md = mod.depth.load();
		const auto pol = md > 0.f ? 1.f : -1.f;
		const auto dAbs = md * pol;
		const auto dRemapped = biased(0.f, dAbs, mod.bias.load(), modSrc);
		const auto mValue = dRemapped * pol;
		return mValue;
	}

	float Param::getValMod() const noexcept
	{
		return valMod.load();
	}

	float Param::getValModDenorm() const noexcept
	{
		return range.convertFrom0to1(valMod.load());
	}

	void Param::setModBias(float b) noexcept
	{
		if (isLocked() || !modulatable)
			return;
		b = BiasEps + b * (1.f - 2.f * BiasEps);
		b = juce::jlimit(BiasEps, 1.f - BiasEps, b);
		mod.bias.store(b);
	}

	float Param::getModBias() const noexcept
	{
		return mod.bias.load();
	}

	void Param::setModulationDefault() noexcept
	{
		setModDepth(0.f);
		setModBias(.5f);
	}

	void Param::setModDepthAbsolute(bool e) noexcept
	{
		modDepthAbsolute = e;
	}

	void Param::setDefaultValue(float norm) noexcept
	{
		valDenormDefault = range.convertFrom0to1(norm);
	}

	void Param::startModulation() noexcept
	{
		valInternal = getValue();
	}

	void Param::modulate(float modSrc) noexcept
	{
		if (!modulatable)
			return;
		valInternal += calcValModOf(modSrc);
	}

	void Param::endModulation(int numChannels) noexcept
	{
		auto y = valInternal;
		if (modulatable)
			y = math::limit(0.f, 1.f, y);
		if (curValMod == y)
			return;
		curValMod = y;
		valMod.store(curValMod);
		callback({ *this, curValMod, numChannels });
	}

	float Param::getDefaultValue() const
	{
		return range.convertTo0to1(valDenormDefault);
	}

	String Param::getName(int) const
	{
		return toString(id);
	}

	// units of param (hz, % etc.)
	String Param::getLabel() const
	{
		return toString(unit);
	}

	// string of norm val
	String Param::getText(float norm, int) const
	{
		return valToStr(range.snapToLegalValue(range.convertFrom0to1(norm)));
	}

	// string to norm val
	float Param::getValueForText(const String& text) const
	{
		const auto val = juce::jlimit(range.start, range.end, strToVal(text));
		return range.convertTo0to1(val);
	}

	// string to denorm val
	float Param::getValForTextDenorm(const String& text) const
	{
		return strToVal(text);
	}

	int Param::getNumSteps() const
	{
		if (range.interval > 0.f)
		{
			const auto numSteps = (range.end - range.start) / range.interval;
			return 1 + static_cast<int>(numSteps);
		}

		return juce::AudioProcessor::getDefaultNumParameterSteps();
	}

	bool Param::isLocked() const noexcept
	{
		return locked.load();
	}

	void Param::setLocked(bool e) noexcept
	{
		locked.store(e);
	}

	void Param::switchLock() noexcept
	{
		setLocked(!isLocked());
	}

	float Param::biased(float start, float end, float bias, float x) noexcept
	{
		const auto r = end - start;
		if (r == 0.f)
			return 0.f;
		const auto a2 = 2.f * bias;
		const auto aM = 1.f - bias;
		const auto aR = r * bias;
		return start + aR * x / (aM - x + a2 * x);
	}
}

namespace param::strToVal
{
	std::function<float(String, const float)> parse()
	{
		return [](const String& txt, const float altVal)
		{
			fx::Parser fx;
			if (fx(txt))
				return fx();

			return altVal;
		};
	}

	StrToValFunc standard(float defaultVal)
	{
		return[p = parse(), defaultVal](const String& txt)
		{
			if (math::stringNegates(txt))
				return defaultVal;
			const auto val = p(txt, defaultVal);
			return val;
		};
	}

	StrToValFunc power()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Power));
			if (math::stringNegates(text))
				return 0.f;
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc solo()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Solo));
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc mute()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Mute));
			const auto val = p(text, 0.f);
			return val > .5f ? 1.f : 0.f;
		};
	}

	StrToValFunc percent()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Percent));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc hz()
	{
		return[p = parse()](const String& txt)
		{
			auto text = txt.trimCharactersAtEnd(toString(Unit::Hz));
			auto multiplier = 1.f;
			if (text.getLastCharacter() == 'k')
			{
				multiplier = 1000.f;
				text = text.dropLastCharacters(1);
			}
			const auto val = p(text, 0.f);
			const auto val2 = val * multiplier;

			return val2;
		};
	}

	StrToValFunc phase()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Degree));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc oct()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
			const auto val = p(text, 0.f);
			return std::round(val);
		};
	}

	StrToValFunc octFloat()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Octaves));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc semi()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Semi));
			const auto val = p(text, 0.f);
			return std::round(val);
		};
	}

	StrToValFunc fine()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Fine));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc ratio()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Ratio));
			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc lrms()
	{
		return [](const String& txt)
		{
			return txt[0] == 'l' ? 0.f : 1.f;
		};
	}

	StrToValFunc freeSync()
	{
		return [](const String& txt)
		{
			return txt[0] == 'f' ? 0.f : 1.f;
		};
	}

	StrToValFunc polarity()
	{
		return [](const String& txt)
		{
			return txt[0] == '0' ? 0.f : 1.f;
		};
	}

	StrToValFunc ms()
	{
		return[p = parse()](const String& txt)
		{
			auto iStr = 0;
			for(auto i = 0; i < txt.length(); ++i)
				if (txt[i] >= 'a' && txt[i] <= 'z' || txt[i] >= 'A' && txt[i] <= 'Z')
				{
					iStr = i;
					i = txt.length();
				}
			const auto text = txt.substring(0, iStr);
			const auto val = p(text, 0.f);
			if(txt.endsWith("sec") || txt.endsWith("sek"))
				return val * 1000.f;
			else
				return val;
		};
	}

	StrToValFunc db()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Decibel));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc voices()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Voices));
			const auto val = p(text, 1.f);
			return val;
		};
	}

	StrToValFunc pan(const Params& params)
	{
		return[p = parse(), &prms = params](const String& txt)
		{
			if (txt == "center" || txt == "centre")
				return 0.f;

			const auto text = txt.trimCharactersAtEnd("MSLR").toLowerCase();
#if PPDHasStereoConfig
			const auto sc = prms[PID::StereoConfig];
			if (sc->getValMod() < .5f)
#endif
			{
				if (txt == "l" || txt == "left")
					return -1.f;
				else if (txt == "r" || txt == "right")
					return 1.f;
			}
#if PPDHasStereoConfig
			else
			{

				if (txt == "m" || txt == "mid")
					return -1.f;
				else if (txt == "s" || txt == "side")
					return 1.f;
			}
#endif

			const auto val = p(text, 0.f);
			return val * .01f;
		};
	}

	StrToValFunc xen()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Xen));
			const auto val = p(text, 0.f);
			return val;
		};
	}

	StrToValFunc note()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.toLowerCase();
			auto val = p(text, -1.f);
			if (val >= 0.f && val < 128.f)
				return val;

			enum pitchclass { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B, Num };
			enum class State { Pitchclass, FlatOrSharp, Parse, numStates };

			auto state = State::Pitchclass;

			for (auto i = 0; i < text.length(); ++i)
			{
				auto chr = text[i];

				if (state == State::Pitchclass)
				{
					if (chr == 'c')
						val = C;
					else if (chr == 'd')
						val = D;
					else if (chr == 'e')
						val = E;
					else if (chr == 'f')
						val = F;
					else if (chr == 'g')
						val = G;
					else if (chr == 'a')
						val = A;
					else if (chr == 'b')
						val = B;
					else
						return 69.f;

					state = State::FlatOrSharp;
				}
				else if (state == State::FlatOrSharp)
				{
					if (chr == '#')
						++val;
					else if (chr == 'b')
						--val;
					else
						--i;

					state = State::Parse;
				}
				else if (state == State::Parse)
				{
					auto newVal = p(text.substring(i), -1.f);
					if (newVal == -1.f)
						return 69.f;
					val += 12 + newVal * 12.f;
					while (val < 0.f)
						val += 12.f;
					return val;
				}
				else
					return 69.f;
			}

			return juce::jlimit(0.f, 127.f, val + 12.f);
		};
	}

#if PPDHasTuningEditor
	StrToValFunc pitch(const Xen& xenManager)
	{
		return[hzFunc = hz(), noteFunc = note(), &xen = xenManager](const String& txt)
		{
			auto freqHz = hzFunc(txt);
			if (freqHz != 0.f)
				return xen.freqHzToNote(freqHz);

			return noteFunc(txt);
		};
	}
#else
	StrToValFunc pitch()
	{
		return[hzFunc = hz(), noteFunc = note()](const String& txt)
		{
			auto freqHz = hzFunc(txt);
			if (freqHz != 0.f)
				return math::freqHzToNote(freqHz);

			return noteFunc(txt);
		};
	}
#endif

	StrToValFunc q()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Q));
			const auto val = p(text, 40.f);
			return val;
		};
	}

	StrToValFunc slope()
	{
		return[p = parse()](const String& txt)
		{
			const auto text = txt.trimCharactersAtEnd(toString(Unit::Slope));
			const auto val = p(text, 0.f);
			return val / 12.f;
		};
	}

	StrToValFunc beats()
	{
		return[p = parse()](const String& txt)
		{
			enum Mode { Beats, Triplet, Dotted, NumModes };
			const auto lastChr = txt[txt.length() - 1];
			const auto mode = lastChr == 't' ? Mode::Triplet : lastChr == '.' ? Mode::Dotted : Mode::Beats;

			const auto text = mode == Mode::Beats ? txt : txt.substring(0, txt.length() - 1);
			auto val = p(text, 1.f / 16.f);
			if (mode == Mode::Triplet)
				val *= 1.666666666667f;
			else if (mode == Mode::Dotted)
				val *= 1.75f;
			return val;
		};
	}

	StrToValFunc legato()
	{
		return[p = parse()](const String& txt)
		{
			if (math::stringNegates(txt))
				return 0.f;
			return p(txt, 0.f);
		};
	}

	StrToValFunc filterType()
	{
		return[p = parse()](const String& txt)
		{
			auto text = txt.toLowerCase();
			if (text == "lp")
				return 0.f;
			else if (text == "hp")
				return 1.f;
			else if (text == "bp")
				return 2.f;
			else if (text == "br")
				return 3.f;
			else if (text == "ap")
				return 4.f;
			else if (text == "ls")
				return 5.f;
			else if (text == "hs")
				return 6.f;
			else if (text == "notch")
				return 7.f;
			else if (text == "bell")
				return 8.f;
			else
				return p(text, 0.f);
		};
	}
}

namespace param::valToStr
{
	ValToStrFunc mute()
	{
		return [](float v) { return v > .5f ? "Mute" : "Not Mute"; };
	}

	ValToStrFunc solo()
	{
		return [](float v) { return v > .5f ? "Solo" : "Not Solo"; };
	}

	ValToStrFunc power()
	{
		return [](float v) { return v > .5f ? "Enabled" : "Disabled"; };
	}

	ValToStrFunc percent()
	{
		return [](float v) { return String(std::round(v * 100.f)) + " " + toString(Unit::Percent); };
	}

	ValToStrFunc hz()
	{
		return [](float v)
			{
				if (v >= 10000.f)
					return String(v * .001).substring(0, 4) + " k" + toString(Unit::Hz);
				else if (v >= 1000.f)
					return String(v * .001).substring(0, 3) + " k" + toString(Unit::Hz);
				else
					return String(v).substring(0, 5) + " " + toString(Unit::Hz);
			};
	}

	ValToStrFunc phase()
	{
		return [](float v) { return String(std::round(v * 180.f)) + " " + toString(Unit::Degree); };
	}

	ValToStrFunc phase360()
	{
		return [](float v) { return String(std::round(v * 360.f)) + " " + toString(Unit::Degree); };
	}

	ValToStrFunc oct()
	{
		return [](float v) { return String(std::round(v)) + " " + toString(Unit::Octaves); };
	}

	ValToStrFunc octFloat()
	{
		return [](float v) { return String(v, 1) + " " + toString(Unit::Octaves); };
	}

	ValToStrFunc semi()
	{
		return [](float v) { return String(std::round(v)) + " " + toString(Unit::Semi); };
	}

	ValToStrFunc fine()
	{
		return [](float v) { return String(std::round(v * 100.f)) + " " + toString(Unit::Fine); };
	}

	ValToStrFunc ratio()
	{
		return [](float v)
			{
				const auto y = static_cast<int>(std::round(v * 100.f));
				return String(100 - y) + " : " + String(y);
			};
	}

	ValToStrFunc lrms()
	{
		return [](float v) { return v > .5f ? String("m/s") : String("l/r"); };
	}

	ValToStrFunc freeSync()
	{
		return [](float v) { return v > .5f ? String("sync") : String("free"); };
	}

	ValToStrFunc polarity()
	{
		return [](float v) { return v > .5f ? String("on") : String("off"); };
	}

	ValToStrFunc ms()
	{
		return [](float v)
			{
				if (v < 100.f)
				{
					v = std::round(v * 100.f) * .01f;
					return String(v) + " ms";
				}
				else if (v < 1000.f)
				{
					v = std::round(v * 10.f) * .1f;
					return String(v) + " ms";
				}
				else
				{
					v = std::round(v * .1f) * .01f;
					return String(v) + " sec";
				}
			};
	}

	ValToStrFunc db()
	{
		return [](float v) { return String(v, 1) + " " + toString(Unit::Decibel); };
	}

	ValToStrFunc empty()
	{
		return [](float) { return String(""); };
	}

	ValToStrFunc voices()
	{
		return [](float v)
			{
				return String(std::round(v)) + toString(Unit::Voices);
			};
	}

	ValToStrFunc pan(const Params& params)
	{
		return [&prms = params](float v)
			{
				if (v == 0.f)
					return String("C");

#if PPDHasStereoConfig
				const auto sc = prms[PID::StereoConfig];
				const auto vm = sc->getValMod();
				const auto isMidSide = vm > .5f;

				if (!isMidSide)
#endif
				{
					if (v == -1.f)
						return String("Left");
					else if (v == 1.f)
						return String("Right");
					else
						return String(std::round(v * 100.f)) + (v < 0.f ? " L" : " R");
				}
#if PPDHasStereoConfig
				else
				{
					if (v == -1.f)
						return String("Mid");
					else if (v == 1.f)
						return String("Side");
					else
						return String(std::round(v * 100.f)) + (v < 0.f ? " M" : " S");
				}
#endif
			};
	}

	ValToStrFunc note()
	{
		return [](float v)
			{
				if (v >= 0.f)
				{
					enum pitchclass { C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B, Num };

					const auto note = static_cast<int>(std::round(v));
					const auto octave = note / 12 - 1;
					const auto noteName = note % 12;
					return math::pitchclassToString(noteName) + String(octave);
				}
				return String("?");
			};
	}

#if PPDHasTuningEditor
	ValToStrFunc xen(const Params& params)
	{
		return [&prms = params](float v)
			{
				const auto& snapParam = prms(PID::XenSnap);
				const auto snap = snapParam.getValMod() > .5f;
				if (snap)
					v = std::round(v);
				else
					v = std::round(v * 100.f) * .01f;
				return String(v) + " " + toString(Unit::Xen);
			};
	}

	ValToStrFunc pitch(const Xen& xenManager, bool showsPitch)
	{
		return [noteFunc = note(), hzFunc = hz(), &xen = xenManager, showsPitch](float v)
		{
			auto hzStr = hzFunc(xen.noteToFreqHz(v));
			if(showsPitch)
				return noteFunc(v) + "; " + hzStr;
			else
				return hzStr;
		};
	}
#else
	ValToStrFunc pitch(bool showsPitch)
	{
		if(showsPitch)
			return [noteFunc = note(), hzFunc = hz()](float v)
			{
				return noteFunc(v) + "; " + hzFunc(math::noteToFreqHz(v));
			};
		else
			return [hzFunc = hz()](float v)
			{
				return hzFunc(math::noteToFreqHz(v));
			};
	}
#endif
	

	ValToStrFunc q()
	{
		return [](float v)
			{
				v = std::round(v * 100.f) * .01f;
				return String(v) + " " + toString(Unit::Q);
			};
	}

	ValToStrFunc slope()
	{
		return [](float v)
			{
				v = std::round(v) * 12.f;
				return String(v) + " " + toString(Unit::Slope);
			};
	}

	ValToStrFunc beats()
	{
		enum Mode { Whole, Triplet, Dotted, NumModes };

		return [](float v)
			{
				if (v == 0.f)
					return String("0");

				const auto denormFloor = math::nextLowestPowTwoX(v);
				const auto denormFrac = v - denormFloor;
				const auto modeVal = denormFrac / denormFloor;
				const auto mode = modeVal < .66f ? Mode::Whole :
					modeVal < .75f ? Mode::Triplet :
					Mode::Dotted;
				String modeStr = mode == Mode::Whole ? String("") :
					mode == Mode::Triplet ? String("t") :
					String(".");

				auto denominator = 1.f / denormFloor;
				auto numerator = 1.f;
				if (denominator < 1.f)
				{
					numerator = denormFloor;
					denominator = 1.f;
				}

				return String(numerator) + " / " + String(denominator) + modeStr;
			};
	}

	ValToStrFunc legato()
	{
		return [](float v)
			{
				return v < .5f ? String("Off") : v < 1.5f ? String("On") : String("On+Sus");
			};
	}

	ValToStrFunc filterType()
	{
		return [](float v)
			{
				auto idx = static_cast<int>(std::round(v));
				switch (idx)
				{
				case 0: return String("LP");
				case 1: return String("HP");
				case 2: return String("BP");
				case 3: return String("BR");
				case 4: return String("AP");
				case 5: return String("LS");
				case 6: return String("HS");
				case 7: return String("Notch");
				case 8: return String("Bell");
				default: return String("");
				}
			};
	}
}

namespace param
{
	// pID, valDenormDefault, range, Unit
	Param* makeParam(PID id, float valDenormDefault = 1.f,
		const Range& range = { 0.f, 1.f }, Unit unit = Unit::Percent, bool modulatable = true)
	{
		ValToStrFunc valToStrFunc;
		StrToValFunc strToValFunc;

		switch (unit)
		{
		case Unit::Power:
			valToStrFunc = valToStr::power();
			strToValFunc = strToVal::power();
			break;
		case Unit::Solo:
			valToStrFunc = valToStr::solo();
			strToValFunc = strToVal::solo();
			break;
		case Unit::Mute:
			valToStrFunc = valToStr::mute();
			strToValFunc = strToVal::mute();
			break;
		case Unit::Decibel:
			valToStrFunc = valToStr::db();
			strToValFunc = strToVal::db();
			break;
		case Unit::Ms:
			valToStrFunc = valToStr::ms();
			strToValFunc = strToVal::ms();
			break;
		case Unit::Percent:
			valToStrFunc = valToStr::percent();
			strToValFunc = strToVal::percent();
			break;
		case Unit::Hz:
			valToStrFunc = valToStr::hz();
			strToValFunc = strToVal::hz();
			break;
		case Unit::Ratio:
			valToStrFunc = valToStr::ratio();
			strToValFunc = strToVal::ratio();
			break;
		case Unit::Polarity:
			valToStrFunc = valToStr::polarity();
			strToValFunc = strToVal::polarity();
			break;
		case Unit::StereoConfig:
			valToStrFunc = valToStr::lrms();
			strToValFunc = strToVal::lrms();
			break;
		case Unit::Octaves:
			valToStrFunc = valToStr::oct();
			strToValFunc = strToVal::oct();
			break;
		case Unit::OctavesFloat:
			valToStrFunc = valToStr::octFloat();
			strToValFunc = strToVal::octFloat();
			break;
		case Unit::Semi:
			valToStrFunc = valToStr::semi();
			strToValFunc = strToVal::semi();
			break;
		case Unit::Fine:
			valToStrFunc = valToStr::fine();
			strToValFunc = strToVal::fine();
			break;
		case Unit::Voices:
			valToStrFunc = valToStr::voices();
			strToValFunc = strToVal::voices();
			break;
		case Unit::Note:
			valToStrFunc = valToStr::note();
			strToValFunc = strToVal::note();
			break;
		case Unit::Q:
			valToStrFunc = valToStr::q();
			strToValFunc = strToVal::q();
			break;
		case Unit::Slope:
			valToStrFunc = valToStr::slope();
			strToValFunc = strToVal::slope();
			break;
		case Unit::Beats:
			valToStrFunc = valToStr::beats();
			strToValFunc = strToVal::beats();
			break;
		case Unit::Legato:
			valToStrFunc = valToStr::legato();
			strToValFunc = strToVal::legato();
			break;
		case Unit::FilterType:
			valToStrFunc = valToStr::filterType();
			strToValFunc = strToVal::filterType();
			break;
		default:
			valToStrFunc = [](float v) { return String(v); };
			strToValFunc = [p = strToVal::parse()](const String& s)
			{
				return p(s, 0.f);
			};
			break;
		}

		return new Param(id, range, valDenormDefault, valToStrFunc, strToValFunc, unit, modulatable);
	}

	// pID, params
	Param* makeParamPan(PID id, const Params& params)
	{
		ValToStrFunc valToStrFunc = valToStr::pan(params);
		StrToValFunc strToValFunc = strToVal::pan(params);

		return new Param(id, { -1.f, 1.f }, 0.f, valToStrFunc, strToValFunc, Unit::Pan, true);
	}

#if PPDHasTuningEditor
	// pID, valDenormDefault, range, Xen
	Param* makeParamPitch(PID id, float valDenormDefault,
		const Range& range, const Xen& xen)
	{
		ValToStrFunc valToStrFunc = valToStr::pitch(xen);
		StrToValFunc strToValFunc = strToVal::pitch(xen);

		return new Param(id, range, valDenormDefault, valToStrFunc, strToValFunc, Unit::Pitch);
	}

	Param* makeParamPitch(PID id, float valDenormDefault, const Xen& xen)
	{
		const auto lowestPitch = math::freqHzToNote2(20.f);
		const auto highestPitch = math::freqHzToNote2(20000.f);
		const auto pitchRange = makeRange::lin(lowestPitch, highestPitch);
		return makeParamPitch(id, valDenormDefault, pitchRange, xen);
	}

	Param* makeParamXen(const Params& params)
	{
		constexpr auto MaxXen = static_cast<float>(arch::MaxXen);
		constexpr auto DefaultXen = 12.f;
		const auto range = makeRange::withCentre(3.f, MaxXen, DefaultXen);
		ValToStrFunc valToStrFunc = valToStr::xen(params);
		StrToValFunc strToValFunc = strToVal::xen();
		return new Param(PID::Xen, range, DefaultXen, valToStrFunc, strToValFunc, Unit::Xen);
	}
#else
	// pID, valDenormDefault, start, end, showsPitch
	Param* makeParamPitch(PID id, float valDenormDefault,
		float start, float end, bool showsPitch)
	{
		ValToStrFunc valToStrFunc = valToStr::pitch(showsPitch);
		StrToValFunc strToValFunc = strToVal::pitch();

		return new Param(id, makeRange::lin(start, end), valDenormDefault, valToStrFunc, strToValFunc, Unit::Pitch, true);
	}

	// pID, valDenormDefault, showsPitch
	Param* makeParamPitch(PID id, float valDenormDefault, bool showsPitch)
	{
		const auto lowestPitch = math::freqHzToNote2(20.f);
		const auto highestPitch = math::freqHzToNote2(20000.f);
		return makeParamPitch(id, valDenormDefault, lowestPitch, highestPitch, showsPitch);
	}
#endif

	Param* makeParam(PID id, float valDenormDefault, const Range& range,
		const ValToStrFunc& valToStrFunc, const StrToValFunc& strToValFunc = strToVal::standard(0.f), bool modulatable = true)
	{
		return new Param(id, range, valDenormDefault, valToStrFunc, strToValFunc, Unit::Custom, modulatable);
	}
	

	// PARAMS

	Params::Params(AudioProcessor& audioProcessor
#if PPDHasTuningEditor
		, const Xen& xen
#endif
	) :
		params(),
		modDepthAbsolute(false)
	{
		{ // HIGH LEVEL PARAMS:
			params.push_back(makeParam(PID::Macro, 1.f, makeRange::lin(0, 1), Unit::Percent, false));
#if PPDHasSidechain
			params.push_back(makeParam(PID::SCGain, 0.f, makeRange::lin(-24, 24), Unit::Decibel));
			params.push_back(makeParam(PID::SCListen, 0.f, makeRange::toggle(), Unit::Power, false));
#endif
#if PPDIsNonlinear
			const auto gainInRange = makeRange::withCentre(-24, 24, 0);
			params.push_back(makeParam(PID::GainIn, 0.f, gainInRange, Unit::Decibel));
			params.push_back(makeParam(PID::UnityGain, 1.f, makeRange::toggle(), Unit::Polarity, false));
#endif
#if PPDIO == PPDIODryWet
			const auto gainRangeCentre = -20.f;
			const auto gainDryRange = makeRange::withCentre(PPDGainDryMin, PPDGainDryMax, gainRangeCentre);
			const auto gainWetRange = makeRange::withCentre(PPDGainWetMin, PPDGainWetMax, gainRangeCentre);
			params.push_back(makeParam(PID::GainDry, PPDGainDryMin, gainDryRange, Unit::Decibel));
			params.push_back(makeParam(PID::GainWet, -18.f, gainWetRange, Unit::Decibel));
#elif PPDIO == PPDIOWetMix
			params.push_back(makeParam(PID::GainWet, 0.f, makeRange::lin(-24, 24), Unit::Decibel));
			params.push_back(makeParam(PID::Mix, 1.f));
#if PPDHasDelta
			params.push_back(makeParam(PID::Delta, 0.f, makeRange::toggle(), Unit::Power));
#endif
#endif
			params.push_back(makeParam(PID::GainOut, 0.f, makeRange::lin(-60, 0), Unit::Decibel));
#if PPDHasStereoConfig
			params.push_back(makeParam(PID::StereoConfig, 0.f, makeRange::toggle(), Unit::StereoConfig));
#endif
#if PPDHasHQ
			params.push_back(makeParam(PID::HQ, 1.f, makeRange::toggle(), Unit::Power));
#endif
#if PPDHasLookahead
			params.push_back(makeParam(PID::Lookahead, 0.f, makeRange::toggle(), Unit::Power));
#endif
#if PPDHasTuningEditor
			// TUNING PARAMS:
			params.push_back(makeParamXen(*this));
			params.push_back(makeParam(PID::XenSnap, 1.f, makeRange::toggle(), Unit::Power));
			params.push_back(makeParam(PID::MasterTune, 440.f, makeRange::withCentre(420.f, 460.f, 440.f), Unit::Hz));
			params.push_back(makeParam(PID::AnchorPitch, 69.f, makeRange::stepped(0.f, 127.f), Unit::Note));
			params.push_back(makeParam(PID::PitchbendRange, 2.f, makeRange::stepped(1.f, 48.f), Unit::Semi));
			for (auto i = 0; i < arch::MaxXen; ++i)
			{
				const auto id = offset(PID::TempC, i);
				params.push_back(makeParam(id, 0.f, makeRange::lin(-1.f, 1.f), Unit::Fine));
			}
#endif
			params.push_back(makeParam(PID::Power, 1.f, makeRange::toggle(), Unit::Power, false));
		}

		// LOW LEVEL PARAMS:		
		// LOW LEVEL PARAMS END

		for (auto param : params)
			audioProcessor.addParameter(param);
	}

	void Params::prepare() noexcept
	{
		for (auto param : params)
			param->prepare();
	}

	void Params::loadPatch(const State& state)
	{
		const String idStr("params/");
		const auto mda = state.get(idStr + "mdabs");
		if (mda != nullptr)
			setModDepthAbsolute(static_cast<int>(*mda) != 0);

		for (auto param : params)
			param->loadPatch(state);
	}

	void Params::savePatch(State& state) const
	{
		for (auto param : params)
			param->savePatch(state);

		const String idStr("params/");
		state.set(idStr + "mdabs", (isModDepthAbsolute() ? 1 : 0));
	}

	int Params::getParamIdx(const String& nameOrID) const
	{
		for (auto p = 0; p < params.size(); ++p)
		{
			const auto pName = toString(params[p]->id);
			if (nameOrID == pName || nameOrID == toID(pName))
				return p;
		}
		return -1;
	}

	size_t Params::numParams() const noexcept
	{
		return params.size();
	}

	void Params::modulate(float modSrc, int numChannels) noexcept
	{
		for (auto i = 1; i < NumParams; ++i)
		{
			auto& p = *params[i];
			p.startModulation();
			p.modulate(modSrc);
			p.endModulation(numChannels);
		}
	}

	Param* Params::operator[](int i) noexcept { return params[i]; }
	const Param* Params::operator[](int i) const noexcept { return params[i]; }
	Param* Params::operator[](PID p) noexcept { return params[static_cast<int>(p)]; }
	const Param* Params::operator[](PID p) const noexcept { return params[static_cast<int>(p)]; }

	Param& Params::operator()(int i) noexcept { return *params[i]; }
	const Param& Params::operator()(int i) const noexcept { return *params[i]; }
	Param& Params::operator()(PID p) noexcept { return *params[static_cast<int>(p)]; }
	const Param& Params::operator()(PID p) const noexcept { return *params[static_cast<int>(p)]; }

	Params::Parameters& Params::data() noexcept
	{
		return params;
	}

	const Params::Parameters& Params::data() const noexcept
	{
		return params;
	}

	bool Params::isModDepthAbsolute() const noexcept
	{
		return modDepthAbsolute.load();
	}

	void Params::setModDepthAbsolute(bool e) noexcept
	{
		modDepthAbsolute.store(e);
		for (auto& p : params)
			p->setModDepthAbsolute(e);
	}

	void Params::switchModDepthAbsolute() noexcept
	{
		setModDepthAbsolute(!isModDepthAbsolute());
	}
}