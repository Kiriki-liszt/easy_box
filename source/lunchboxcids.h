//------------------------------------------------------------------------
// Copyright(c) 2023 yg331.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"


namespace yg331 {
	//------------------------------------------------------------------------
	static const Steinberg::FUID klunchboxProcessorUID(0x9B16F1C8, 0x51FB52B3, 0xBD5826BA, 0x9E94BA87);
	static const Steinberg::FUID klunchboxControllerUID(0xEE704E30, 0xAD1E5A10, 0xA5754B21, 0xFE1E9419);

#define lunchboxVST3Category "Fx"

	//------------------------------------------------------------------------


	enum {
		kParamBypass = 0,

		kParamInput,
		kParamOutput,

		kParamDrive,
		kParamLowcut,
		kParamAir,
		kParamHigh,
		kParamFocus,
		kParamBody,
		kParamLow,
		kParamIntensity,
		kParamSharpness,
		kParamDepth,
		kParamListen,
		kParamComp,
		kParamSpeed,
		kParamAttack,
		kParamGate,
		kParamInflate,
		kParamSafe,

		kParamInVuPPM,
		kParamOutVuPPM,
		kParamDeEssVuPPM,
		kParamCompVuPPM
	};

	const bool BypassInit = false, LowcutInit = false, ListenInit = false, AttackInit = false, SafeInit = false;

	const double InputInit = 0.5,
		OutputInit = 0.5,

		DriveInit = 0.2,

		AirInit = 0.0,
		HighInit = 0.5,
		FocusInit = 0.5,
		BodyInit = 0.5,
		LowInit = 0.5,
		IntensityInit = 0.5,
		SharpnessInit = 0.5,
		DepthInit = 0.5,

		CompInit = 0.2,
		SpeedInit = 0.6,

		GateInit = 0.0,
		InflateInit = 0.2,


		InVuPPMInit = 0.0,
		OutVuPPMInit = 0.0,
		DeEssVuPPMInit = 1.0,
		CompVuPPMInit = 1.0;
} // namespace yg331
