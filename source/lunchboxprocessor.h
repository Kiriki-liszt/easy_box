//------------------------------------------------------------------------
// Copyright(c) 2023 yg331.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "lunchboxcids.h"

#include <math.h>
#define M_E        2.71828182845904523536   // e
#define M_LOG2E    1.44269504088896340736   // log2(e)
#define M_LOG10E   0.434294481903251827651  // log10(e)
#define M_LN2      0.693147180559945309417  // ln(2)
#define M_LN10     2.30258509299404568402   // ln(10)
#define M_PI       3.14159265358979323846   // pi
#define M_PI_2     1.57079632679489661923   // pi/2
#define M_PI_4     0.785398163397448309616  // pi/4
#define M_1_PI     0.318309886183790671538  // 1/pi
#define M_2_PI     0.636619772367581343076  // 2/pi
#define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)

using namespace Steinberg;

namespace yg331 {

	//------------------------------------------------------------------------
	//  lunchboxProcessor
	//------------------------------------------------------------------------
	class lunchboxProcessor : public Steinberg::Vst::AudioEffect
	{
	public:
		lunchboxProcessor();
		~lunchboxProcessor() SMTG_OVERRIDE;

		// Create function
		static Steinberg::FUnknown* createInstance(void* /*context*/)
		{
			return (Steinberg::Vst::IAudioProcessor*)new lunchboxProcessor;
		}

		//--- ---------------------------------------------------------------------
		// AudioEffect overrides:
		//--- ---------------------------------------------------------------------
		/** Called at first after constructor */
		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;

		/** Called at the end before destructor */
		Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

		Steinberg::tresult PLUGIN_API setBusArrangements(
			Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
			Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts
		) SMTG_OVERRIDE {
			// we only support one in and output bus and these busses must have the same number of channels
			if (numIns == 1 && numOuts == 1 && Vst::SpeakerArr::getChannelCount(inputs[0]) == 2 && Vst::SpeakerArr::getChannelCount(outputs[0]) == 2)
				return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
			return kResultFalse;
		};

		/** Switch the Plug-in on/off */
		Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

		/** Will be called before any process call */
		Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;

		/** Asks if a given sample size is supported see SymbolicSampleSizes. */
		Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

		/** Here we go...the process call */
		Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;

		/** For persistence */
		Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;

		template <typename SampleType>
		void processChannel9(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processEQ(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processDeBess(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processComp(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processGate(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processInflator(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processInput(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		template <typename SampleType>
		void processOutput(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames, int32 precision);

		template <typename SampleType>
		void processBypass(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames);

		inline void setCoeffs(double Fs);
		

		Vst::Sample64 norm_to_gain(Vst::Sample64 plainValue) {
			return exp(log(10.0) * (24.0 * plainValue - 12.0) / 20.0);
		}
		Vst::Sample64 VuPPMconvert(Vst::Sample64 plainValue, Vst::Sample64 Min, Vst::Sample64 Max, Vst::Sample64 Mid)
		{
			double dB = 20.0 * log10(plainValue);
			double normValue;

			if (dB > Max) normValue = 1.0;
			else if (dB > Mid) normValue = (dB - (2 * Mid - Max)) / (2 * Max - 2 * Mid);
			else if (dB > Min) normValue = (dB - Min) / (2 * Mid - 2 * Min);
			else normValue = 0.0;

			return normValue;
		}


		//------------------------------------------------------------------------
	protected:

		uint32 fpdL = 1.0;
		uint32 fpdR = 1.0; 

		// Channel9 + add Highpass
		Vst::Sample64 iirSampleLA = 0.0;
		Vst::Sample64 iirSampleRA = 0.0;
		Vst::Sample64 iirSampleLB = 0.0;
		Vst::Sample64 iirSampleRB = 0.0;
		Vst::Sample64 lastSampleAL = 0.0;
		Vst::Sample64 lastSampleBL = 0.0;
		Vst::Sample64 lastSampleCL = 0.0;
		Vst::Sample64 lastSampleAR = 0.0;
		Vst::Sample64 lastSampleBR = 0.0;
		Vst::Sample64 lastSampleCR = 0.0;
		Vst::Sample64 biquadA[15] = { 0, };
		Vst::Sample64 biquadB[15] = { 0, };
		Vst::Sample64 iirAmount = 0.005832;
		Vst::Sample64 threshold = 0.33362176;
		Vst::Sample64 cutoff = 28811.0;
		bool          flip_channel9 = false;

		// EQ
		Vst::Sample64 z_1k2[3] = { 0, }, p_1k2[3] = { 0, };
		Vst::Sample64 x_1k2_L[3] = { 0, }, y_1k2_L[3] = { 0, };
		Vst::Sample64 x_1k2_R[3] = { 0, }, y_1k2_R[3] = { 0, };
		Vst::Sample64 K_1k2 = 0.0, K_1k2_2 = 0.0, Q_1k2 = 1.5;

		Vst::Sample64 z_10[3] = { 0, }, p_10[3] = { 0, };
		Vst::Sample64 x_10_L[3] = { 0, }, y_10_L[3] = { 0, };
		Vst::Sample64 x_10_R[3] = { 0, }, y_10_R[3] = { 0, };
		Vst::Sample64 z_40[3] = { 0, }, p_40[3] = { 0, };
		Vst::Sample64 x_40_L[3] = { 0, }, y_40_L[3] = { 0, };
		Vst::Sample64 x_40_R[3] = { 0, }, y_40_R[3] = { 0, };
		Vst::Sample64 z_160[3] = { 0, },   p_160[3] = { 0, };
		Vst::Sample64 x_160_L[3] = { 0, }, y_160_L[3] = { 0, };
		Vst::Sample64 x_160_R[3] = { 0, }, y_160_R[3] = { 0, };
		Vst::Sample64 z_640[3] = { 0, },   p_640[3] = { 0, };
		Vst::Sample64 x_640_L[3] = { 0, }, y_640_L[3] = { 0, };
		Vst::Sample64 x_640_R[3] = { 0, }, y_640_R[3] = { 0, };
		Vst::Sample64 z_2k5[3] = { 0, },   p_2k5[3] = { 0, };
		Vst::Sample64 x_2k5_L[3] = { 0, }, y_2k5_L[3] = { 0, };
		Vst::Sample64 x_2k5_R[3] = { 0, }, y_2k5_R[3] = { 0, };
		Vst::Sample64 z_20k[3] = { 0, },   p_20k[3] = { 0, };
		Vst::Sample64 x_20k_L[3] = { 0, }, y_20k_L[3] = { 0, };
		Vst::Sample64 x_20k_R[3] = { 0, }, y_20k_R[3] = { 0, };

		// DeBess
		Vst::Sample64 sL[41] = { 0, }, mL[41] = { 0, }, cL[41] = { 0, };
		Vst::Sample64 ratioAL = 1.0;
		Vst::Sample64 ratioBL = 1.0;
		Vst::Sample64 iirSampleAL = 0.0;
		Vst::Sample64 iirSampleBL = 0.0;

		Vst::Sample64 sR[41] = { 0, }, mR[41] = { 0, }, cR[41] = { 0, };
		Vst::Sample64 ratioAR = 1.0;
		Vst::Sample64 ratioBR = 1.0;
		Vst::Sample64 iirSampleAR = 0.0;
		Vst::Sample64 iirSampleBR = 0.0;
		bool          flip_DeBess = false;

		// MeowMu
		Vst::Sample64 muVaryL = 1.0;
		Vst::Sample64 muAttackL = 0.0;
		Vst::Sample64 muNewSpeedL = 0.0;
		Vst::Sample64 muSpeedAL = 10000;
		Vst::Sample64 muSpeedBL = 10000;
		Vst::Sample64 muCoefficientAL = 1.0;
		Vst::Sample64 muCoefficientBL = 1.0;
		Vst::Sample64 previousL = 0.0;

		Vst::Sample64 muVaryR = 1.0;
		Vst::Sample64 muAttackR = 0.0;
		Vst::Sample64 muNewSpeedR = 0.0;
		Vst::Sample64 muSpeedAR = 10000;
		Vst::Sample64 muSpeedBR = 10000;
		Vst::Sample64 muCoefficientAR = 1.0;
		Vst::Sample64 muCoefficientBR = 1.0;
		Vst::Sample64 previousR = 0.0;
		bool          flip_MeowMu = false;

		//begin Gate
		bool WasNegativeL = false;
		int32 ZeroCrossL = 0;
		Vst::Sample64 gaterollerL = 0.0;
		Vst::Sample64 gateL = 0.0;

		bool WasNegativeR = false;
		int32 ZeroCrossR = 0;
		Vst::Sample64 gaterollerR = 0.0;
		Vst::Sample64 gateR = 0.0;
		//end Gate

		// Parameters
		bool          bParamBypass = BypassInit;
		Vst::Sample32 fParamInput = InputInit;
		Vst::Sample32 fParamOutput = OutputInit;
		Vst::Sample32 fParamDrive = DriveInit;
		bool          bParamLowcut = LowcutInit;
		Vst::Sample32 fParamAir = AirInit;
		Vst::Sample32 fParamHigh = HighInit;
		Vst::Sample32 fParamFocus = FocusInit;
		Vst::Sample32 fParamBody = BodyInit;
		Vst::Sample32 fParamLow = LowInit;
		Vst::Sample32 fParamIntensity = IntensityInit;
		Vst::Sample32 fParamSharpness = SharpnessInit;
		Vst::Sample32 fParamDepth = DepthInit;
		bool          bParamListen = ListenInit;
		Vst::Sample32 fParamComp = CompInit;
		Vst::Sample32 fParamSpeed = SpeedInit;
		bool          bParamAttack = AttackInit;
		Vst::Sample32 fParamGate = GateInit;
		Vst::Sample32 fParamInflate = InflateInit;
		bool          bParamSafe = SafeInit;

		Vst::Sample32 fParamInVuPPM = 0.0;
		Vst::Sample32 fParamOutVuPPM = 0.0;
		Vst::Sample32 fParamDeEssVuPPM = 1.0;
		Vst::Sample32 fParamCompVuPPM = 1.0;
		Vst::Sample32 fParamInVuPPMOld = 0.0;
		Vst::Sample32 fParamOutVuPPMOld = 0.0;
		Vst::Sample32 fParamDeEssVuPPMOld = 1.0;
		Vst::Sample32 fParamCompVuPPMOld = 1.0;

	};
	//------------------------------------------------------------------------
} // namespace yg331
