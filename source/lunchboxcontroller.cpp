//------------------------------------------------------------------------
// Copyright(c) 2023 yg331.
//------------------------------------------------------------------------

#include "lunchboxcontroller.h"
#include "lunchboxcids.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "pluginterfaces/base/ustring.h"
#include "base/source/fstreamer.h"

#include "public.sdk/source/vst/vsteditcontroller.h"

using namespace Steinberg;

namespace yg331 {


	//------------------------------------------------------------------------
	// lunchboxController Implementation
	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::initialize(FUnknown* context)
	{
		// Here the Plug-in will be instantiated

		//---do not forget to call parent ------
		tresult result = EditControllerEx1::initialize(context);
		if (result != kResultOk)
		{
			return result;
		}

		int32 stepCount;
		int32 flags;
		int32 tag;
		Vst::ParamValue defaultVal;
		Vst::ParamValue minPlain;
		Vst::ParamValue maxPlain;
		Vst::ParamValue defaultPlain;

		flags = Vst::ParameterInfo::kCanAutomate;
		minPlain = -12.0;
		maxPlain = 12.0;
		defaultPlain = 0.0;

		tag = kParamInput;
		auto* gainParamIn = new SliderParameter(USTRING("Input"), tag, STR16("dB"), minPlain, maxPlain, defaultPlain, 0, flags);
		parameters.addParameter(gainParamIn);
		tag = kParamOutput;
		auto* gainParamOut = new SliderParameter(USTRING("Output"), tag, STR16("dB"), minPlain, maxPlain, defaultPlain, 0, flags);
		parameters.addParameter(gainParamOut);

		flags = Vst::ParameterInfo::kIsReadOnly;
		minPlain = -60;
		maxPlain = 0;
		defaultPlain = -18;

		tag = kParamInVuPPM;
		auto* InVuPPM = new VuPPMParameter(USTRING("InVuPPM"), flags, tag, STR16("dB"), minPlain, maxPlain, defaultPlain);
		parameters.addParameter(InVuPPM);
		tag = kParamOutVuPPM;
		auto* OutVuPPM = new VuPPMParameter(USTRING("OutVuPPM"), flags, tag, STR16("dB"), minPlain, maxPlain, defaultPlain);
		parameters.addParameter(OutVuPPM);

		minPlain = -12;
		maxPlain = 0;
		defaultPlain = -6;

		tag = kParamCompVuPPM;
		auto* GRVuPPM = new VuPPMParameter(STR16("CompVuPPM"), flags, tag, STR16("dB"), minPlain, maxPlain, defaultPlain);
		parameters.addParameter(GRVuPPM);
		tag = kParamDeEssVuPPM;
		auto* DeEssVuPPM = new VuPPMParameter(STR16("DeEssVuPPM"), flags, tag, STR16("dB"), minPlain, maxPlain, defaultPlain);
		parameters.addParameter(DeEssVuPPM);


		tag = kParamDrive;
		stepCount = 0;
		defaultVal = DriveInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Drive"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamAir;
		stepCount = 0;
		defaultVal = AirInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Air"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamHigh;
		stepCount = 0;
		defaultVal = HighInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("High"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamFocus;
		stepCount = 0;
		defaultVal = FocusInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Focus"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamBody;
		stepCount = 0;
		defaultVal = BodyInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Body"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamLow;
		stepCount = 0;
		defaultVal = LowInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Low"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamIntensity;
		stepCount = 0;
		defaultVal = IntensityInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Intensity"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamSharpness;
		stepCount = 0;
		defaultVal = SharpnessInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Sharpness"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamDepth;
		stepCount = 0;
		defaultVal = DepthInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Depth"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamComp;
		stepCount = 0;
		defaultVal = CompInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Comp"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamSpeed;
		stepCount = 0;
		defaultVal = SpeedInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Speed"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamGate;
		stepCount = 0;
		defaultVal = GateInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Gate"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamInflate;
		stepCount = 0;
		defaultVal = InflateInit;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Inflate"), nullptr, stepCount, defaultVal, flags, tag);

		tag = kParamLowcut;
		stepCount = 1;
		defaultVal = 0;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Lowcut"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamListen;
		stepCount = 1;
		defaultVal = 0;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Listen"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamAttack;
		stepCount = 1;
		defaultVal = 0;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Attack"), nullptr, stepCount, defaultVal, flags, tag);
		tag = kParamSafe;
		stepCount = 1;
		defaultVal = 0;
		flags = Vst::ParameterInfo::kCanAutomate;
		parameters.addParameter(STR16("Safe"), nullptr, stepCount, defaultVal, flags, tag);


		tag = kParamBypass;
		stepCount = 1;
		defaultVal = 0;
		flags = Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass;
		parameters.addParameter(STR16("Bypass"), nullptr, stepCount, defaultVal, flags, tag);



		return result;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::terminate()
	{
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

		//---do not forget to call parent ------
		return EditControllerEx1::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::setComponentState(IBStream* state)
	{
		// Here you get the state of the component (Processor part)
		if (!state)
			return kResultFalse;

		IBStreamer streamer(state, kLittleEndian);

		float savedInput = 0.f;
		if (streamer.readFloat(savedInput) == false)
			return kResultFalse;
		setParamNormalized(kParamInput, savedInput);

		float savedOutput = 0.f;
		if (streamer.readFloat(savedOutput) == false)
			return kResultFalse;
		setParamNormalized(kParamOutput, savedOutput);

		float savedDrive = 0.f;
		if (streamer.readFloat(savedDrive) == false)
			return kResultFalse;
		setParamNormalized(kParamDrive, savedDrive);

		float savedAir = 0.f;
		if (streamer.readFloat(savedAir) == false)
			return kResultFalse;
		setParamNormalized(kParamAir, savedAir);

		float savedHigh = 0.f;
		if (streamer.readFloat(savedHigh) == false)
			return kResultFalse;
		setParamNormalized(kParamHigh, savedHigh);

		float savedFocus = 0.f;
		if (streamer.readFloat(savedFocus) == false)
			return kResultFalse;
		setParamNormalized(kParamFocus, savedFocus);

		float savedBody = 0.f;
		if (streamer.readFloat(savedBody) == false)
			return kResultFalse;
		setParamNormalized(kParamBody, savedBody);

		float savedLow = 0.f;
		if (streamer.readFloat(savedLow) == false)
			return kResultFalse;
		setParamNormalized(kParamLow, savedLow);

		float savedIntensity = 0.f;
		if (streamer.readFloat(savedIntensity) == false)
			return kResultFalse;
		setParamNormalized(kParamIntensity, savedIntensity);

		float savedSharpness = 0.f;
		if (streamer.readFloat(savedSharpness) == false)
			return kResultFalse;
		setParamNormalized(kParamSharpness, savedSharpness);

		float savedDepth = 0.f;
		if (streamer.readFloat(savedDepth) == false)
			return kResultFalse;
		setParamNormalized(kParamDepth, savedDepth);

		float savedComp = 0.f;
		if (streamer.readFloat(savedComp) == false)
			return kResultFalse;
		setParamNormalized(kParamComp, savedComp);

		float savedSpeed = 0.f;
		if (streamer.readFloat(savedSpeed) == false)
			return kResultFalse;
		setParamNormalized(kParamSpeed, savedSpeed);


		float savedGate = 0.f;
		if (streamer.readFloat(savedGate) == false)
			return kResultFalse;
		setParamNormalized(kParamGate, savedGate);

		float savedInflate = 0.f;
		if (streamer.readFloat(savedInflate) == false)
			return kResultFalse;
		setParamNormalized(kParamInflate, savedInflate);

		int32 savedLowcut = 0;
		if (streamer.readInt32(savedLowcut) == false)
			return kResultFalse;
		setParamNormalized(kParamLowcut, savedLowcut);

		int32 savedListen = 0;
		if (streamer.readInt32(savedListen) == false)
			return kResultFalse;
		setParamNormalized(kParamListen , savedListen);

		int32 savedAttack = 0;
		if (streamer.readInt32(savedAttack) == false)
			return kResultFalse;
		setParamNormalized(kParamAttack, savedAttack);

		int32 savedSafe = 0;
		if (streamer.readInt32(savedSafe) == false)
			return kResultFalse;
		setParamNormalized(kParamSafe, savedSafe);

		int32 savedBypass = 0;
		if (streamer.readInt32(savedBypass) == false)
			return kResultFalse;
		setParamNormalized(kParamBypass, savedBypass);

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::setState(IBStream* state)
	{
		// Here you get the state of the controller

		return kResultTrue;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::getState(IBStream* state)
	{
		// Here you are asked to deliver the state of the controller (if needed)
		// Note: the real state of your plug-in is saved in the processor

		return kResultTrue;
	}

	//------------------------------------------------------------------------
	IPlugView* PLUGIN_API lunchboxController::createView(FIDString name)
	{
		// Here the Host wants to open your editor (if you have one)
		if (FIDStringsEqual(name, Vst::ViewType::kEditor))
		{
			// create your editor here and return a IPlugView ptr of it
			auto* view = new VSTGUI::VST3Editor(this, "view", "lunchboxeditor.uidesc");
			return view;
		}
		return nullptr;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::setParamNormalized(Vst::ParamID tag, Vst::ParamValue value)
	{
		// called by host to update your parameters
		tresult result = EditControllerEx1::setParamNormalized(tag, value);
		return result;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::getParamStringByValue(Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
	{
		// called by host to get a string for given normalized value of a specific parameter
		// (without having to set the value!)
		return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxController::getParamValueByString(Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
	{
		// called by host to get a normalized value from a string representation of a specific parameter
		// (without having to set the value!)
		return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
	}



	//------------------------------------------------------------------------
	// GainParameter Implementation
	//------------------------------------------------------------------------
	SliderParameter::SliderParameter(
		const Vst::TChar* title,
		int32 tag,
		const Vst::TChar* units,
		Vst::ParamValue minPlain,
		Vst::ParamValue maxPlain,
		Vst::ParamValue defaultValuePlain,
		int32 stepCount,
		int32 flags,
		Vst::UnitID unitID
	)
	{
		UString(info.title, str16BufferSize(Vst::String128)).assign(title);
		if (units)
			UString(info.units, str16BufferSize(Vst::String128)).assign(units);

		setMin(minPlain);
		setMax(maxPlain);

		info.flags = flags;
		info.id = tag;
		info.stepCount = stepCount;
		info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
		info.unitId = Vst::kRootUnitId;
	}
	//------------------------------------------------------------------------
	void SliderParameter::toString(Vst::ParamValue normValue, Vst::String128 string) const
	{
		Vst::ParamValue plainValue = SliderParameter::toPlain(normValue);

		char text[32];
		snprintf(text, 32, "%.2f", plainValue);

		Steinberg::UString(string, 128).fromAscii(text);
	}
	//------------------------------------------------------------------------
	bool SliderParameter::fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const
	{
		String wrapper((Vst::TChar*)string); // don't know buffer size here!
		Vst::ParamValue plainValue;
		if (wrapper.scanFloat(plainValue))
		{
			normValue = SliderParameter::toNormalized(plainValue);
			return true;
		}
		return false;

	}
	//------------------------------------------------------------------------
	VuPPMParameter::VuPPMParameter(
		const Vst::TChar* title,
		int32 flags,
		int32 tag,
		const Vst::TChar* units,
		Vst::ParamValue minPlain,
		Vst::ParamValue maxPlain,
		Vst::ParamValue defaultValuePlain,
		Vst::UnitID unitID
	)
	{

		UString(info.title, str16BufferSize(Vst::String128)).assign(title);
		if (units)
			UString(info.units, str16BufferSize(Vst::String128)).assign(units);

		setMin(minPlain);
		setMax(maxPlain);

		setMid(defaultValuePlain);

		info.stepCount = 0;
		info.defaultNormalizedValue = valueNormalized = toNormalized(defaultValuePlain);
		info.flags = flags;
		info.id = tag;
		info.unitId = unitID;
	}

	Vst::ParamValue VuPPMParameter::toPlain(Vst::ParamValue normValue) const
	{
		Vst::ParamValue plainValue;
		/*
		normValue	: dB		: gain
		1.0 ~ 0.0	:   0 ~ -36	: 1.0 ~ 0.015849
		1.0 ~ 0.5	:   0 ~ -18 : 1.0 ~ 0.12589254117941673
		0.5 ~ 0.0	: -18 ~ -60 : 0.12589254117941673 ~
		1.0 ~ 0.0	: +24 ~ -60

		1. normValue -> dB
		1.0 ~ 0.5	: y = (36 * y) - 36;
		0.5 ~ 0.0	: y = (84 * x) - 60;
		*/
		if (normValue > 0.5) plainValue = (2 * (getMax() - getMid()) * normValue) + (2 * getMid() - getMax());
		else plainValue = (2 * (getMid() - getMin()) * normValue) + getMin();

		return plainValue;
	}

	Vst::ParamValue VuPPMParameter::toNormalized(Vst::ParamValue plainValue) const
	{
		double dB = 20.0 * log10(plainValue);
		double normValue;
		double Max = getMax();
		double Mid = getMid();
		double Min = getMin();
		if (dB > Max) normValue = 1.0;
		else if (dB > Mid) normValue = (dB - (2 * Mid - Max)) / (2 * Max - 2 * Mid);
		else if (dB > Min) normValue = (dB - Min) / (2 * Mid - 2 * Min);
		else normValue = 0.0;

		return normValue;
	}

	//------------------------------------------------------------------------
	void VuPPMParameter::toString(Vst::ParamValue normValue, Vst::String128 string) const
	{
		Vst::ParamValue plainValue = VuPPMParameter::toPlain(normValue);

		char text[32];
		snprintf(text, 32, "%.2f", plainValue);

		Steinberg::UString(string, 128).fromAscii(text);
	}

	//------------------------------------------------------------------------
	bool VuPPMParameter::fromString(const Vst::TChar* string, Vst::ParamValue& normValue) const
	{
		String wrapper((Vst::TChar*)string); // don't know buffer size here!
		Vst::ParamValue plainValue;
		if (wrapper.scanFloat(plainValue))
		{
			normValue = VuPPMParameter::toNormalized(plainValue);
			return true;
		}
		return false;
	}


	//------------------------------------------------------------------------
} // namespace yg331
