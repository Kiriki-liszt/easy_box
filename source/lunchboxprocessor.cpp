//------------------------------------------------------------------------
// Copyright(c) 2023 yg331.
//------------------------------------------------------------------------

#include "lunchboxprocessor.h"


#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"
#include "public.sdk/source/vst/vsthelpers.h"


using namespace Steinberg;

namespace yg331 {
	//------------------------------------------------------------------------
	// lunchboxProcessor
	//------------------------------------------------------------------------
	lunchboxProcessor::lunchboxProcessor()
	{
		//--- set the wanted controller for our processor
		setControllerClass(klunchboxControllerUID);
	}

	//------------------------------------------------------------------------
	lunchboxProcessor::~lunchboxProcessor()
	{}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::initialize(FUnknown* context)
	{
		// Here the Plug-in will be instantiated

		//---always initialize the parent-------
		tresult result = AudioEffect::initialize(context);
		// if everything Ok, continue
		if (result != kResultOk)
		{
			return result;
		}

		//--- create Audio IO ------
		addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
		addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

		/* If you don't need an event bus, you can remove the next line */
		addEventInput(STR16("Event In"), 1);

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::terminate()
	{
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

		//---do not forget to call parent ------
		return AudioEffect::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::setActive(TBool state)
	{
		//--- called when the Plug-in is enable/disable (On/Off) -----

		fParamInVuPPM = 0.0;
		fParamOutVuPPM = 0.0;
		fParamDeEssVuPPM = 1.0;
		fParamCompVuPPM = 1.0;
		fParamInVuPPMOld = 0.0;
		fParamOutVuPPMOld = 0.0;
		fParamDeEssVuPPMOld = 1.0;
		fParamCompVuPPMOld = 1.0;
		return AudioEffect::setActive(state);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::process(Vst::ProcessData& data)
	{
		Vst::IParameterChanges* paramChanges = data.inputParameterChanges;
		if (paramChanges)
		{
			int32 numParamsChanged = paramChanges->getParameterCount();
			for (int32 index = 0; index < numParamsChanged; index++)
			{
				Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(index);
				if (paramQueue)
				{
					Vst::ParamValue value;
					int32 sampleOffset;
					int32 numPoints = paramQueue->getPointCount();

					/*/*/
					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
						switch (paramQueue->getParameterId()) {
						case kParamInput:   	fParamInput = (float)value;		break;
						case kParamOutput:  	fParamOutput = (float)value;	break;
						case kParamDrive:   	fParamDrive = (float)value;		break;
						case kParamAir:     	fParamAir = (float)value;	break;
						case kParamHigh:    	fParamHigh = (float)value;	break;
						case kParamFocus:   	fParamFocus = (float)value;		break;
						case kParamBody:    	fParamBody = (float)value;	break;
						case kParamLow:	    	fParamLow = (float)value;	break;
						case kParamIntensity:	fParamIntensity = (float)value;		break;
						case kParamSharpness:	fParamSharpness = (float)value;		break;
						case kParamDepth:   	fParamDepth = (float)value;		break;
						case kParamComp:    	fParamComp = (float)value;	break;
						case kParamSpeed:   	fParamSpeed = (float)value;		break;
						case kParamGate:    	fParamGate = (float)value;	break;
						case kParamInflate:		fParamInflate = (float)value;	break;
						case kParamLowcut:  	bParamLowcut = (value > 0.5f);	break;
						case kParamListen:  	bParamListen = (value > 0.5f);	break;
						case kParamAttack:  	bParamAttack = (value > 0.5f);	break;
						case kParamSafe:    	bParamSafe = (value > 0.5f);	break;
						case kParamBypass:  	bParamBypass = (value > 0.5f);	break;
						}
					}
				}
			}
		}

		//--- Here you have to implement your processing



		if (data.numInputs == 0 || data.numOutputs == 0) {
			return kResultOk;
		}

		//---get audio buffers----------------
		uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
		void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
		void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);
		double getSampleRate = processSetup.sampleRate;


		//---check if silence---------------
		// normally we have to check each channel (simplification)
		if (data.inputs[0].silenceFlags != 0) // if flags is not zero => then it means that we have silent!
		{
			// in the example we are applying a gain to the input signal
			// As we know that the input is only filled with zero, the output will be then filled with zero too!

			data.outputs[0].silenceFlags = 0;

			if (data.inputs[0].silenceFlags & (uint64)1) { // Left
				if (in[0] != out[0])
					memset(out[0], 0, sampleFramesSize); // this is faster than applying a gain to each sample!
				data.outputs[0].silenceFlags |= (uint64)1 << (uint64)0;
			}

			if (data.inputs[0].silenceFlags & (uint64)2) { // Right
				if (in[1] != out[1])
					memset(out[1], 0, sampleFramesSize); // this is faster than applying a gain to each sample!
				data.outputs[0].silenceFlags |= (uint64)1 << (uint64)1;
			}

			if (data.inputs[0].silenceFlags & (uint64)3) {
				return kResultOk;
			}
		}

		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		fParamInVuPPM = 0.f;
		fParamOutVuPPM = 0.f;
		fParamDeEssVuPPM = 1.f;
		fParamCompVuPPM = 1.f;
		
		//---in bypass mode outputs should be like inputs-----
		if (bParamBypass)
		{
			if (in[0] != out[0]) { memcpy(out[0], in[0], sampleFramesSize); }
			if (in[1] != out[1]) { memcpy(out[1], in[1], sampleFramesSize); }

			if (data.symbolicSampleSize == Vst::kSample32) {
				processBypass<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
			}
			else if (data.symbolicSampleSize == Vst::kSample64) {
				processBypass<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
			}
		}
		else
		{
			if (data.symbolicSampleSize == Vst::kSample32) {
				processInput<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processChannel9<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processEQ<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processDeBess<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processComp<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processInflator<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples);
				processOutput<Vst::Sample32>((Vst::Sample32**)in, getSampleRate, data.numSamples, Vst::kSample32);
				memcpy(out[0], in[0], sampleFramesSize);
				memcpy(out[1], in[1], sampleFramesSize);
			}
			else if (data.symbolicSampleSize == Vst::kSample64) {
				processInput<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processChannel9<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processEQ<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processDeBess<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processComp<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processInflator<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples);
				processOutput<Vst::Sample64>((Vst::Sample64**)in, getSampleRate, data.numSamples, Vst::kSample64);
				memcpy(out[0], in[0], sampleFramesSize);
				memcpy(out[1], in[1], sampleFramesSize);
			}
		}

		//---3) Write outputs parameter changes-----------
		Vst::IParameterChanges* outParamChanges = data.outputParameterChanges;
		// a new value of VuMeter will be send to the host
		// (the host will send it back in sync to our controller for updating our editor)
		if (outParamChanges)
		{
			int32 index = 0;
			Vst::IParamValueQueue* paramQueue = outParamChanges->addParameterData(kParamInVuPPM, index);
			if (paramQueue)
			{
				int32 index2 = 0;
				paramQueue->addPoint(0, fParamInVuPPM, index2);
			}

			index = 0;
			paramQueue = outParamChanges->addParameterData(kParamOutVuPPM, index);
			if (paramQueue)
			{
				int32 index2 = 0;
				paramQueue->addPoint(0, fParamOutVuPPM, index2);
			}

			index = 0;
			paramQueue = outParamChanges->addParameterData(kParamDeEssVuPPM, index);
			if (paramQueue)
			{
				int32 index2 = 0;
				paramQueue->addPoint(0, fParamDeEssVuPPM, index2);
			}

			index = 0;
			paramQueue = outParamChanges->addParameterData(kParamCompVuPPM, index);
			if (paramQueue)
			{
				int32 index2 = 0;
				paramQueue->addPoint(0, fParamCompVuPPM, index2);
			}

		}
		fParamInVuPPMOld = fParamInVuPPM;
		fParamOutVuPPMOld = fParamOutVuPPM;
		fParamDeEssVuPPMOld = fParamDeEssVuPPM;
		fParamCompVuPPMOld = fParamCompVuPPM;

		return kResultOk;
	}


	template <typename SampleType>
	void lunchboxProcessor::processChannel9(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames)
	{
		SampleType* in1 = inputs[0];
		SampleType* in2 = inputs[1];

		double overallscale = 1.0;
		overallscale /= 44100.0;
		overallscale *= getSampleRate;
		double localiirAmount = iirAmount / overallscale;
		double localthreshold = threshold; //we've learned not to try and adjust threshold for sample rate
		double density = fParamDrive; //0-2, originally at "* 2.0"
		double phattity = density - 1.0;
		if (density > 1.0) density = 1.0; //max out at full wet for Spiral aspect
		if (phattity < 0.0) phattity = 0.0; //
		double nonLin = 5.0 - density; //number is smaller for more intense, larger for more subtle
		biquadB[0] = biquadA[0] = cutoff / getSampleRate;
		biquadA[1] = 1.618033988749894848204586;
		biquadB[1] = 0.618033988749894848204586;

		double K = tan(M_PI * biquadA[0]); //lowpass
		double norm = 1.0 / (1.0 + K / biquadA[1] + K * K);
		biquadA[2] = K * K * norm;
		biquadA[3] = 2.0 * biquadA[2];
		biquadA[4] = biquadA[2];
		biquadA[5] = 2.0 * (K * K - 1.0) * norm;
		biquadA[6] = (1.0 - K / biquadA[1] + K * K) * norm;

		K = tan(M_PI * biquadA[0]);
		norm = 1.0 / (1.0 + K / biquadB[1] + K * K);
		biquadB[2] = K * K * norm;
		biquadB[3] = 2.0 * biquadB[2];
		biquadB[4] = biquadB[2];
		biquadB[5] = 2.0 * (K * K - 1.0) * norm;
		biquadB[6] = (1.0 - K / biquadB[1] + K * K) * norm;

		while (--sampleFrames >= 0)
		{
			double inputSampleL = *in1;
			double inputSampleR = *in2;

			double tempSample;

			if (biquadA[0] < 0.49999) {
				tempSample = biquadA[2] * inputSampleL + biquadA[3] * biquadA[7] + biquadA[4] * biquadA[8] - biquadA[5] * biquadA[9] - biquadA[6] * biquadA[10];
				biquadA[8] = biquadA[7]; biquadA[7] = inputSampleL; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSampleL = tempSample;
				biquadA[10] = biquadA[9]; biquadA[9] = inputSampleL; //DF1 left
				tempSample = biquadA[2] * inputSampleR + biquadA[3] * biquadA[11] + biquadA[4] * biquadA[12] - biquadA[5] * biquadA[13] - biquadA[6] * biquadA[14];
				biquadA[12] = biquadA[11]; biquadA[11] = inputSampleR; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSampleR = tempSample;
				biquadA[14] = biquadA[13]; biquadA[13] = inputSampleR; //DF1 right
			}

			double dielectricScaleL = fabs(2.0 - ((inputSampleL + nonLin) / nonLin));
			double dielectricScaleR = fabs(2.0 - ((inputSampleR + nonLin) / nonLin));

			if (flip_channel9)
			{
				if (fabs(iirSampleLA) < 1.18e-37) iirSampleLA = 0.0;
				iirSampleLA = (iirSampleLA * (1.0 - (localiirAmount * dielectricScaleL))) + (inputSampleL * localiirAmount * dielectricScaleL);
				inputSampleL = inputSampleL - iirSampleLA;
				if (fabs(iirSampleRA) < 1.18e-37) iirSampleRA = 0.0;
				iirSampleRA = (iirSampleRA * (1.0 - (localiirAmount * dielectricScaleR))) + (inputSampleR * localiirAmount * dielectricScaleR);
				inputSampleR = inputSampleR - iirSampleRA;
			}
			else
			{
				if (fabs(iirSampleLB) < 1.18e-37) iirSampleLB = 0.0;
				iirSampleLB = (iirSampleLB * (1.0 - (localiirAmount * dielectricScaleL))) + (inputSampleL * localiirAmount * dielectricScaleL);
				inputSampleL = inputSampleL - iirSampleLB;
				if (fabs(iirSampleRB) < 1.18e-37) iirSampleRB = 0.0;
				iirSampleRB = (iirSampleRB * (1.0 - (localiirAmount * dielectricScaleR))) + (inputSampleR * localiirAmount * dielectricScaleR);
				inputSampleR = inputSampleR - iirSampleRB;
			}
			//highpass section
			double drySampleL = inputSampleL;
			double drySampleR = inputSampleR;

			if (inputSampleL > 1.0) inputSampleL = 1.0;
			if (inputSampleL < -1.0) inputSampleL = -1.0;
			double phatSampleL = sin(inputSampleL * 1.57079633);
			inputSampleL *= 1.2533141373155;
			//clip to 1.2533141373155 to reach maximum output, or 1.57079633 for pure sine 'phat' version

			double distSampleL = sin(inputSampleL * fabs(inputSampleL)) / ((fabs(inputSampleL) == 0.0) ? 1 : fabs(inputSampleL));

			inputSampleL = distSampleL; //purest form is full Spiral
			if (density < 1.0) inputSampleL = (drySampleL * (1 - density)) + (distSampleL * density); //fade Spiral aspect
			if (phattity > 0.0) inputSampleL = (inputSampleL * (1 - phattity)) + (phatSampleL * phattity); //apply original Density on top

			if (inputSampleR > 1.0) inputSampleR = 1.0;
			if (inputSampleR < -1.0) inputSampleR = -1.0;
			double phatSampleR = sin(inputSampleR * 1.57079633);
			inputSampleR *= 1.2533141373155;
			//clip to 1.2533141373155 to reach maximum output, or 1.57079633 for pure sine 'phat' version

			double distSampleR = sin(inputSampleR * fabs(inputSampleR)) / ((fabs(inputSampleR) == 0.0) ? 1 : fabs(inputSampleR));

			inputSampleR = distSampleR; //purest form is full Spiral
			if (density < 1.0) inputSampleR = (drySampleR * (1 - density)) + (distSampleR * density); //fade Spiral aspect
			if (phattity > 0.0) inputSampleR = (inputSampleR * (1 - phattity)) + (phatSampleR * phattity); //apply original Density on top

			//begin L
			double clamp = (lastSampleBL - lastSampleCL) * 0.381966011250105;
			clamp -= (lastSampleAL - lastSampleBL) * 0.6180339887498948482045;
			clamp += inputSampleL - lastSampleAL; //regular slew clamping added

			lastSampleCL = lastSampleBL;
			lastSampleBL = lastSampleAL;
			lastSampleAL = inputSampleL; //now our output relates off lastSampleB

			if (clamp > localthreshold)
				inputSampleL = lastSampleBL + localthreshold;
			if (-clamp > localthreshold)
				inputSampleL = lastSampleBL - localthreshold;

			lastSampleAL = (lastSampleAL * 0.381966011250105) + (inputSampleL * 0.6180339887498948482045); //split the difference between raw and smoothed for buffer
			//end L

			//begin R
			clamp = (lastSampleBR - lastSampleCR) * 0.381966011250105;
			clamp -= (lastSampleAR - lastSampleBR) * 0.6180339887498948482045;
			clamp += inputSampleR - lastSampleAR; //regular slew clamping added

			lastSampleCR = lastSampleBR;
			lastSampleBR = lastSampleAR;
			lastSampleAR = inputSampleR; //now our output relates off lastSampleB

			if (clamp > localthreshold)
				inputSampleR = lastSampleBR + localthreshold;
			if (-clamp > localthreshold)
				inputSampleR = lastSampleBR - localthreshold;

			lastSampleAR = (lastSampleAR * 0.381966011250105) + (inputSampleR * 0.6180339887498948482045); //split the difference between raw and smoothed for buffer
			//end R

			flip_channel9 = !flip_channel9;

			if (biquadB[0] < 0.49999) {
				tempSample = biquadB[2] * inputSampleL + biquadB[3] * biquadB[7] + biquadB[4] * biquadB[8] - biquadB[5] * biquadB[9] - biquadB[6] * biquadB[10];
				biquadB[8] = biquadB[7]; biquadB[7] = inputSampleL; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSampleL = tempSample;
				biquadB[10] = biquadB[9]; biquadB[9] = inputSampleL; //DF1 left
				tempSample = biquadB[2] * inputSampleR + biquadB[3] * biquadB[11] + biquadB[4] * biquadB[12] - biquadB[5] * biquadB[13] - biquadB[6] * biquadB[14];
				biquadB[12] = biquadB[11]; biquadB[11] = inputSampleR; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSampleR = tempSample;
				biquadB[14] = biquadB[13]; biquadB[13] = inputSampleR; //DF1 right
			}

			*in1 = inputSampleL;
			*in2 = inputSampleR;

			in1++;
			in2++;
		}
	}

	template <typename SampleType>
	void lunchboxProcessor::processEQ(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames) {
		SampleType* in1 = (SampleType*)inputs[0];
		SampleType* in2 = (SampleType*)inputs[1];

		Vst::Sample64 x[6], g[5], pg[5];
		Vst::Sample64 g_10, pg_10;
		Vst::Sample64 g_40, pg_40;
		Vst::Sample64 g_160, pg_160;
		Vst::Sample64 g_640, pg_640;
		Vst::Sample64 g_2k5, pg_2k5;
		Vst::Sample64 g_20k, pg_20k;

		Vst::Sample64 peakGain = (12.0 * fParamFocus - 6.0);
		Vst::Sample64 V_1k2 = pow(10, abs(peakGain) / 20);
		Vst::Sample64 QK, VQK;
		if (peakGain > 0.0) {
			QK = 1 / Q_1k2 * K_1k2;
			VQK = V_1k2 / Q_1k2 * K_1k2;
		}
		else {
			QK = V_1k2 / Q_1k2 * K_1k2;
			VQK = 1 / Q_1k2 * K_1k2;
		}
		Vst::Sample64 norm_1k2 = 1 / (1 + QK + K_1k2_2);
		z_1k2[0] = (1 + VQK + K_1k2_2) * norm_1k2;
		z_1k2[1] = 2 * (K_1k2_2 - 1) * norm_1k2;
		z_1k2[2] = (1 - VQK + K_1k2_2) * norm_1k2;
		p_1k2[0] = 1.0;
		p_1k2[1] = z_1k2[1];
		p_1k2[2] = (1 - QK + K_1k2_2) * norm_1k2;

		if (bParamLowcut) {
			x[0] = 0.0; // 10hz
			x[1] = 0.0; // 40hz
		}
		else {
			x[0] = 0.5; // 10hz
			x[1] = 0.5; // 40hz
		}
		x[2] = fParamLow;
		x[3] = fParamBody;
		x[4] = fParamHigh;
		x[5] = fParamAir;

		for (int i = 0; i < 5; i++) {
			if (x[i] > 0.5)
			{
				g[i] = 0.5f * 56.2f / (5.56f + (4011.4f * exp(-7.4746f * x[i]) - 0.54573f));
				pg[i] = 1.f;
			}
			else if (x[i] >= 0.25)
			{
				g[i] = 0.5f * 56.2f / (5.56f + (500 - 810.f * (x[i] - 0.25f) * 2));
				pg[i] = 1.f;
			}
			else
			{
				g[i] = 0.5f * 56.2f / (5.56f + (500));
				pg[i] = (x[i] * 4);
			}
		}
		if (x[5] > 0.5)
			g_20k = 0.5 * 56.2 / (5.56 + (3258.2 * exp(-7.4126 * x[5]) - 1.8466));
		else
			g_20k = 0.5 * 56.2 / (5.56 + (500.0 - 823.6 * x[5]));
		pg_20k = 1.0;

		g_10 = g[0]; pg_10 = pg[0];
		g_40 = g[1]; pg_40 = pg[1];
		g_160 = g[2]; pg_160 = pg[2];
		g_640 = g[3]; pg_640 = pg[3];
		g_2k5 = g[4]; pg_2k5 = pg[4];


		Vst::Sample64 dcGain = g_10 + g_40 + g_160 + g_640 + g_2k5 + g_20k;
		Vst::Sample64 globalGain = 0.398 / dcGain;

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;

			Vst::Sample64 dataOutL = 0.0;
			Vst::Sample64 dataOutR = 0.0;

			/*
			dataOutL += (y_1k2_L[0] * pg_1k2 + inputSampleL) * g_1k2;
			dataOutR += (y_1k2_R[0] * pg_1k2 + inputSampleR) * g_1k2;
			*/

			x_10_L[0] = inputSampleL;
			x_40_L[0] = inputSampleL;
			x_160_L[0] = inputSampleL;
			x_640_L[0] = inputSampleL;
			x_2k5_L[0] = inputSampleL;
			x_20k_L[0] = inputSampleL;

			// 10Hz
			y_10_L[0] = x_10_L[0] * z_10[0] + x_10_L[1] * z_10[1] + x_10_L[2] * z_10[2] - y_10_L[1] * p_10[1] - y_10_L[2] * p_10[2];
			x_10_L[2] = x_10_L[1];  x_10_L[1] = x_10_L[0];  y_10_L[2] = y_10_L[1];  y_10_L[1] = y_10_L[0];

			// 40Hz
			y_40_L[0] = x_40_L[0] * z_40[0] + x_40_L[1] * z_40[1] + x_40_L[2] * z_40[2] - y_40_L[1] * p_40[1] - y_40_L[2] * p_40[2];
			x_40_L[2] = x_40_L[1];  x_40_L[1] = x_40_L[0];  y_40_L[2] = y_40_L[1];  y_40_L[1] = y_40_L[0];

			// 160Hz
			y_160_L[0] = x_160_L[0] * z_160[0] + x_160_L[1] * z_160[1] + x_160_L[2] * z_160[2] - y_160_L[1] * p_160[1] - y_160_L[2] * p_160[2];
			x_160_L[2] = x_160_L[1];  x_160_L[1] = x_160_L[0];  y_160_L[2] = y_160_L[1];  y_160_L[1] = y_160_L[0];

			// 640Hz
			y_640_L[0] = x_640_L[0] * z_640[0] + x_640_L[1] * z_640[1] + x_640_L[2] * z_640[2] - y_640_L[1] * p_640[1] - y_640_L[2] * p_640[2];
			x_640_L[2] = x_640_L[1];  x_640_L[1] = x_640_L[0];  y_640_L[2] = y_640_L[1];  y_640_L[1] = y_640_L[0];

			// 2500Hz
			y_2k5_L[0] = x_2k5_L[0] * z_2k5[0] + x_2k5_L[1] * z_2k5[1] + x_2k5_L[2] * z_2k5[2] - y_2k5_L[1] * p_2k5[1] - y_2k5_L[2] * p_2k5[2];
			x_2k5_L[2] = x_2k5_L[1];  x_2k5_L[1] = x_2k5_L[0];  y_2k5_L[2] = y_2k5_L[1];  y_2k5_L[1] = y_2k5_L[0];

			// 20kHz
			y_20k_L[0] = x_20k_L[0] * z_20k[0] + x_20k_L[1] * z_20k[1] + x_20k_L[2] * z_20k[2] - y_20k_L[1] * p_20k[1] - y_20k_L[2] * p_20k[2];
			x_20k_L[2] = x_20k_L[1];  x_20k_L[1] = x_20k_L[0];  y_20k_L[2] = y_20k_L[1];  y_20k_L[1] = y_20k_L[0];

			dataOutL += (y_10_L[0] * pg_10 + inputSampleL) * g_10;
			dataOutL += (y_40_L[0] * pg_40 + inputSampleL) * g_40;
			dataOutL += (y_160_L[0] * pg_160 + inputSampleL) * g_160;
			dataOutL += (y_640_L[0] * pg_640 + inputSampleL) * g_640;
			dataOutL += (y_2k5_L[0] * pg_2k5 + inputSampleL) * g_2k5;
			dataOutL += (y_20k_L[0] * pg_20k + inputSampleL) * g_20k;

			x_10_R[0] = inputSampleR;
			x_40_R[0] = inputSampleR;
			x_160_R[0] = inputSampleR;
			x_640_R[0] = inputSampleR;
			x_2k5_R[0] = inputSampleR;
			x_20k_R[0] = inputSampleR;

			// 10Hz
			y_10_R[0] = x_10_R[0] * z_10[0] + x_10_R[1] * z_10[1] + x_10_R[2] * z_10[2] - y_10_R[1] * p_10[1] - y_10_R[2] * p_10[2];
			x_10_R[2] = x_10_R[1];  x_10_R[1] = x_10_R[0];  y_10_R[2] = y_10_R[1];  y_10_R[1] = y_10_R[0];

			// 40Hz
			y_40_R[0] = x_40_R[0] * z_40[0] + x_40_R[1] * z_40[1] + x_40_R[2] * z_40[2] - y_40_R[1] * p_40[1] - y_40_R[2] * p_40[2];
			x_40_R[2] = x_40_R[1];  x_40_R[1] = x_40_R[0];  y_40_R[2] = y_40_R[1];  y_40_R[1] = y_40_R[0];

			// 160Hz
			y_160_R[0] = x_160_R[0] * z_160[0] + x_160_R[1] * z_160[1] + x_160_R[2] * z_160[2] - y_160_R[1] * p_160[1] - y_160_R[2] * p_160[2];
			x_160_R[2] = x_160_R[1];  x_160_R[1] = x_160_R[0];  y_160_R[2] = y_160_R[1];  y_160_R[1] = y_160_R[0];

			// 640Hz
			y_640_R[0] = x_640_R[0] * z_640[0] + x_640_R[1] * z_640[1] + x_640_R[2] * z_640[2] - y_640_R[1] * p_640[1] - y_640_R[2] * p_640[2];
			x_640_R[2] = x_640_R[1];  x_640_R[1] = x_640_R[0];  y_640_R[2] = y_640_R[1];  y_640_R[1] = y_640_R[0];

			// 2500Hz
			y_2k5_R[0] = x_2k5_R[0] * z_2k5[0] + x_2k5_R[1] * z_2k5[1] + x_2k5_R[2] * z_2k5[2] - y_2k5_R[1] * p_2k5[1] - y_2k5_R[2] * p_2k5[2];
			x_2k5_R[2] = x_2k5_R[1];  x_2k5_R[1] = x_2k5_R[0];  y_2k5_R[2] = y_2k5_R[1];  y_2k5_R[1] = y_2k5_R[0];

			// 20kHz
			y_20k_R[0] = x_20k_R[0] * z_20k[0] + x_20k_R[1] * z_20k[1] + x_20k_R[2] * z_20k[2] - y_20k_R[1] * p_20k[1] - y_20k_R[2] * p_20k[2];
			x_20k_R[2] = x_20k_R[1];  x_20k_R[1] = x_20k_R[0];  y_20k_R[2] = y_20k_R[1];  y_20k_R[1] = y_20k_R[0];

			dataOutR += (y_10_R[0] * pg_10 + inputSampleR) * g_10;
			dataOutR += (y_40_R[0] * pg_40 + inputSampleR) * g_40;
			dataOutR += (y_160_R[0] * pg_160 + inputSampleR) * g_160;
			dataOutR += (y_640_R[0] * pg_640 + inputSampleR) * g_640;
			dataOutR += (y_2k5_R[0] * pg_2k5 + inputSampleR) * g_2k5;
			dataOutR += (y_20k_R[0] * pg_20k + inputSampleR) * g_20k;

			dataOutL = dataOutL * globalGain;
			dataOutR = dataOutR * globalGain;

			x_1k2_L[0] = dataOutL;
			x_1k2_R[0] = dataOutR;
			// 1200Hz
			y_1k2_L[0] = x_1k2_L[0] * z_1k2[0] + x_1k2_L[1] * z_1k2[1] + x_1k2_L[2] * z_1k2[2] - y_1k2_L[1] * p_1k2[1] - y_1k2_L[2] * p_1k2[2];
			x_1k2_L[2] = x_1k2_L[1];  x_1k2_L[1] = x_1k2_L[0];  y_1k2_L[2] = y_1k2_L[1];  y_1k2_L[1] = y_1k2_L[0];
			// 1200Hz
			y_1k2_R[0] = x_1k2_R[0] * z_1k2[0] + x_1k2_R[1] * z_1k2[1] + x_1k2_R[2] * z_1k2[2] - y_1k2_R[1] * p_1k2[1] - y_1k2_R[2] * p_1k2[2];
			x_1k2_R[2] = x_1k2_R[1];  x_1k2_R[1] = x_1k2_R[0];  y_1k2_R[2] = y_1k2_R[1];  y_1k2_R[1] = y_1k2_R[0];
			dataOutL = y_1k2_L[0];
			dataOutR = y_1k2_R[0];


			*in1 = dataOutL;
			*in2 = dataOutR;

			in1++;
			in2++;
		}
		return;
	}

	template <typename SampleType>
	void lunchboxProcessor::processDeBess(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames) {
		SampleType* in1 = inputs[0];
		SampleType* in2 = inputs[1];

		Vst::Sample64 tmp = 1.0; /*/ VuPPM /*/

		Vst::Sample64 overallscale = 1.0;
		overallscale /= 44100.0;
		overallscale *= getSampleRate;

		Vst::Sample64 intensity = pow(fParamIntensity, 5) * (8192 / overallscale);
		Vst::Sample64 sharpness = fParamSharpness * 40.0;
		if (sharpness < 2) sharpness = 2;
		Vst::Sample64 speed = 0.1 / sharpness;
		Vst::Sample64 depth = 1.0 / ((1.0 - fParamDepth) + 0.0001);
		Vst::Sample64 iirAmount = 0.5; //Filter 
		bool monitoring = bParamListen;

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;

			Vst::Sample64 drySampleL = inputSampleL;
			Vst::Sample64 drySampleR = inputSampleR;

			sL[0] = inputSampleL; //set up so both [0] and [1] will be input sample
			sR[0] = inputSampleR; //set up so both [0] and [1] will be input sample
			//we only use the [1] so this is just where samples come in
			for (int x = sharpness; x > 0; x--) {
				sL[x] = sL[x - 1];
				sR[x] = sR[x - 1];
			} //building up a set of slews

			mL[1] = (sL[1] - sL[2]) * ((sL[1] - sL[2]) / 1.3);
			mR[1] = (sR[1] - sR[2]) * ((sR[1] - sR[2]) / 1.3);
			for (int x = sharpness - 1; x > 1; x--) {
				mL[x] = (sL[x] - sL[x + 1]) * ((sL[x - 1] - sL[x]) / 1.3);
				mR[x] = (sR[x] - sR[x + 1]) * ((sR[x - 1] - sR[x]) / 1.3);
			} //building up a set of slews of slews

			Vst::Sample64 senseL = fabs(mL[1] - mL[2]) * sharpness * sharpness;
			Vst::Sample64 senseR = fabs(mR[1] - mR[2]) * sharpness * sharpness;
			for (int x = sharpness - 1; x > 0; x--) {
				Vst::Sample64 multL = fabs(mL[x] - mL[x + 1]) * sharpness * sharpness;
				if (multL < 1.0) senseL *= multL;
				Vst::Sample64 multR = fabs(mR[x] - mR[x + 1]) * sharpness * sharpness;
				if (multR < 1.0) senseR *= multR;
			} //sense is slews of slews times each other

			senseL = 1.0 + (intensity * intensity * senseL);
			if (senseL > intensity) { senseL = intensity; }
			senseR = 1.0 + (intensity * intensity * senseR);
			if (senseR > intensity) { senseR = intensity; }

			if (flip_DeBess) {
				iirSampleAL = (iirSampleAL * (1 - iirAmount)) + (inputSampleL * iirAmount);
				iirSampleAR = (iirSampleAR * (1 - iirAmount)) + (inputSampleR * iirAmount);
				ratioAL = (ratioAL * (1.0 - speed)) + (senseL * speed);
				ratioAR = (ratioAR * (1.0 - speed)) + (senseR * speed);
				if (ratioAL > depth) ratioAL = depth;
				if (ratioAR > depth) ratioAR = depth;
				if (ratioAL > 1.0) inputSampleL = iirSampleAL + ((inputSampleL - iirSampleAL) / ratioAL);
				if (ratioAR > 1.0) inputSampleR = iirSampleAR + ((inputSampleR - iirSampleAR) / ratioAR);
			}
			else {
				iirSampleBL = (iirSampleBL * (1 - iirAmount)) + (inputSampleL * iirAmount);
				iirSampleBR = (iirSampleBR * (1 - iirAmount)) + (inputSampleR * iirAmount);
				ratioBL = (ratioBL * (1.0 - speed)) + (senseL * speed);
				ratioBR = (ratioBR * (1.0 - speed)) + (senseR * speed);
				if (ratioBL > depth) ratioBL = depth;
				if (ratioBR > depth) ratioBR = depth;
				if (ratioAL > 1.0) inputSampleL = iirSampleBL + ((inputSampleL - iirSampleBL) / ratioBL);
				if (ratioAR > 1.0) inputSampleR = iirSampleBR + ((inputSampleR - iirSampleBR) / ratioBR);
			}
			flip_DeBess = !flip_DeBess;

			if (tmp > (inputSampleL / drySampleL)) tmp = (inputSampleL / drySampleL);
			if (tmp > (inputSampleR / drySampleR)) tmp = (inputSampleR / drySampleR);

			if (monitoring) {
				inputSampleL = drySampleL - inputSampleL;
				inputSampleR = drySampleR - inputSampleR;
			}
			//sense monitoring

			*in1 = inputSampleL;
			*in2 = inputSampleR;

			in1++;
			in2++;
		}
		fParamDeEssVuPPM = VuPPMconvert(tmp, -12.0, 0.0, -6.0);
		return;
	}

	template <typename SampleType>
	void lunchboxProcessor::processComp(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames)
	{
		SampleType* in1 = inputs[0];
		SampleType* in2 = inputs[1];

		Vst::Sample64 tmp = 1.0; /*/ VuPPM /*/

		Vst::Sample64 overallscale = 2.0;
		overallscale /= 44100.0;
		overallscale *= getSampleRate;

		Vst::Sample64 threshold = 1.001 - (1.0 - pow(1.0 - fParamComp, 3));
		Vst::Sample64 muMakeupGain = sqrt(1.0 / threshold);
		muMakeupGain = (muMakeupGain + sqrt(muMakeupGain)) / 2.0;
		muMakeupGain = sqrt(muMakeupGain);
		Vst::Sample64 outGain = sqrt(muMakeupGain);
		//gain settings around threshold
		Vst::Sample64 release = pow((1.15 - fParamSpeed), 5) * 32768.0;
		release /= overallscale;
		// Vst::Sample64 fastest = sqrt(release);
		Vst::Sample64 fastest = sqrt(release);
		//speed settings around release
		Vst::Sample64 coefficient;
		Vst::Sample64 squaredSampleL;
		Vst::Sample64 squaredSampleR;

		// µ µ µ µ µ µ µ µ µ µ µ µ is the kitten song o/~

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;

			Vst::Sample64 drySampleL = inputSampleL;
			Vst::Sample64 drySampleR = inputSampleR;

			inputSampleL *= exp(log(10.0) * (12.0) / 20.0);
			inputSampleR *= exp(log(10.0) * (12.0) / 20.0);

			if (fabs(inputSampleL) > fabs(previousL)) squaredSampleL = previousL * previousL;
			else squaredSampleL = inputSampleL * inputSampleL;
			previousL = inputSampleL;
			// inputSampleL *= muMakeupGain;

			if (fabs(inputSampleR) > fabs(previousR)) squaredSampleR = previousR * previousR;
			else squaredSampleR = inputSampleR * inputSampleR;
			previousR = inputSampleR;
			// inputSampleR *= muMakeupGain;

			//adjust coefficients for L
			if (flip_MeowMu)
			{
				if (fabs(squaredSampleL) > threshold)
				{
					muVaryL = threshold / fabs(squaredSampleL);
					muAttackL = sqrt(fabs(muSpeedAL));
					if (bParamAttack) muAttackL *= 2.0;
					else muAttackL *= 5.0;
					muCoefficientAL = muCoefficientAL * (muAttackL - 1.0);
					if (muVaryL < threshold)
					{
						muCoefficientAL = muCoefficientAL + threshold;
					}
					else
					{
						muCoefficientAL = muCoefficientAL + muVaryL;
					}
					muCoefficientAL = muCoefficientAL / muAttackL;
				}
				else
				{
					muCoefficientAL = muCoefficientAL * ((muSpeedAL * muSpeedAL) - 1.0);
					muCoefficientAL = muCoefficientAL + 1.0;
					muCoefficientAL = muCoefficientAL / (muSpeedAL * muSpeedAL);
				}
				muNewSpeedL = muSpeedAL * (muSpeedAL - 1);
				muNewSpeedL = muNewSpeedL + fabs(squaredSampleL * release) + fastest;
				muSpeedAL = muNewSpeedL / muSpeedAL;
			}
			else
			{
				if (fabs(squaredSampleL) > threshold)
				{
					muVaryL = threshold / fabs(squaredSampleL);
					muAttackL = sqrt(fabs(muSpeedBL));
					if (bParamAttack) muAttackL *= 2.0;
					else muAttackL *= 5.0;
					muCoefficientBL = muCoefficientBL * (muAttackL - 1);
					if (muVaryL < threshold)
					{
						muCoefficientBL = muCoefficientBL + threshold;
					}
					else
					{
						muCoefficientBL = muCoefficientBL + muVaryL;
					}
					muCoefficientBL = muCoefficientBL / muAttackL;
				}
				else
				{
					muCoefficientBL = muCoefficientBL * ((muSpeedBL * muSpeedBL) - 1.0);
					muCoefficientBL = muCoefficientBL + 1.0;
					muCoefficientBL = muCoefficientBL / (muSpeedBL * muSpeedBL);
				}
				muNewSpeedL = muSpeedBL * (muSpeedBL - 1);
				muNewSpeedL = muNewSpeedL + fabs(squaredSampleL * release) + fastest;
				muSpeedBL = muNewSpeedL / muSpeedBL;
			}
			//got coefficients, adjusted speeds for L

			//adjust coefficients for R
			if (flip_MeowMu)
			{
				if (fabs(squaredSampleR) > threshold)
				{
					muVaryR = threshold / fabs(squaredSampleR);
					muAttackR = sqrt(fabs(muSpeedAR));
					if (bParamAttack) muAttackR *= 2.0;
					else muAttackR *= 5.0;
					muCoefficientAR = muCoefficientAR * (muAttackR - 1.0);
					if (muVaryR < threshold)
					{
						muCoefficientAR = muCoefficientAR + threshold;
					}
					else
					{
						muCoefficientAR = muCoefficientAR + muVaryR;
					}
					muCoefficientAR = muCoefficientAR / muAttackR;
				}
				else
				{
					muCoefficientAR = muCoefficientAR * ((muSpeedAR * muSpeedAR) - 1.0);
					muCoefficientAR = muCoefficientAR + 1.0;
					muCoefficientAR = muCoefficientAR / (muSpeedAR * muSpeedAR);
				}
				muNewSpeedR = muSpeedAR * (muSpeedAR - 1);
				muNewSpeedR = muNewSpeedR + fabs(squaredSampleR * release) + fastest;
				muSpeedAR = muNewSpeedR / muSpeedAR;
			}
			else
			{
				if (fabs(squaredSampleR) > threshold)
				{
					muVaryR = threshold / fabs(squaredSampleR);
					muAttackR = sqrt(fabs(muSpeedBR));
					if (bParamAttack) muAttackR *= 2.0;
					else muAttackR *= 5.0;
					muCoefficientBR = muCoefficientBR * (muAttackR - 1);
					if (muVaryR < threshold)
					{
						muCoefficientBR = muCoefficientBR + threshold;
					}
					else
					{
						muCoefficientBR = muCoefficientBR + muVaryR;
					}
					muCoefficientBR = muCoefficientBR / muAttackR;
				}
				else
				{
					muCoefficientBR = muCoefficientBR * ((muSpeedBR * muSpeedBR) - 1.0);
					muCoefficientBR = muCoefficientBR + 1.0;
					muCoefficientBR = muCoefficientBR / (muSpeedBR * muSpeedBR);
				}
				muNewSpeedR = muSpeedBR * (muSpeedBR - 1);
				muNewSpeedR = muNewSpeedR + fabs(squaredSampleR * release) + fastest;
				muSpeedBR = muNewSpeedR / muSpeedBR;
			}
			//got coefficients, adjusted speeds for R

			if (flip_MeowMu)
			{
				coefficient = (muCoefficientAL + pow(muCoefficientAL, 2)) / 2.0;
				inputSampleL *= coefficient;
				coefficient = (muCoefficientAR + pow(muCoefficientAR, 2)) / 2.0;
				inputSampleR *= coefficient;
			}
			else
			{
				coefficient = (muCoefficientBL + pow(muCoefficientBL, 2)) / 2.0;
				inputSampleL *= coefficient;
				coefficient = (muCoefficientBR + pow(muCoefficientBR, 2)) / 2.0;
				inputSampleR *= coefficient;
			}
			//applied compression with vari-vari-µ-µ-µ-µ-µ-µ-is-the-kitten-song o/~
			//applied gain correction to control output level- tends to constrain sound rather than inflate it
			flip_MeowMu = !flip_MeowMu;

			inputSampleL *= exp(log(10.0) * (-12.0) / 20.0);
			inputSampleR *= exp(log(10.0) * (-12.0) / 20.0);

			if (tmp > (inputSampleL / drySampleL)) tmp = (inputSampleL / drySampleL);
			if (tmp > (inputSampleR / drySampleR)) tmp = (inputSampleR / drySampleR);
			if (inputSampleL == drySampleL) tmp = 1.0;
			if (inputSampleR == drySampleR) tmp = 1.0;

			*in1 = inputSampleL;
			*in2 = inputSampleR;

			in1++;
			in2++;
		}
		fParamCompVuPPM = VuPPMconvert(tmp, -12.0, 0.0, -6.0);
		return;
	}


	template <typename SampleType>
	void lunchboxProcessor::processGate(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames)
	{
		float* in1 = inputs[0];
		float* in2 = inputs[1];

		double overallscale = 1.0;
		overallscale /= 44100.0;

		//begin Gate
		double onthreshold = (pow(fParamGate, 3) / 3) + 0.00018;
		double offthreshold = onthreshold * 1.1;
		double release = 0.028331119964586;
		double absmax = 220.9;
		//speed to be compensated w.r.t sample rate
		//end Gate

		while (--sampleFrames >= 0)
		{
			double inputSampleL = *in1;
			double inputSampleR = *in2;

			//begin Gate
			if (inputSampleL > 0.0)
			{
				if (WasNegativeL == true) ZeroCrossL = absmax * 0.3;
				WasNegativeL = false;
			}
			else {
				ZeroCrossL += 1; WasNegativeL = true;
			}

			if (inputSampleR > 0.0)
			{
				if (WasNegativeR == true) ZeroCrossR = absmax * 0.3;
				WasNegativeR = false;
			}
			else {
				ZeroCrossR += 1; WasNegativeR = true;
			}

			if (ZeroCrossL > absmax) ZeroCrossL = absmax;
			if (ZeroCrossR > absmax) ZeroCrossR = absmax;

			if (gateL == 0.0)
			{
				//if gate is totally silent
				if (fabs(inputSampleL) > onthreshold)
				{
					if (gaterollerL == 0.0) gaterollerL = ZeroCrossL;
					else gaterollerL -= release;
					// trigger from total silence only- if we're active then signal must clear offthreshold
				}
				else gaterollerL -= release;
			}
			else {
				//gate is not silent but closing
				if (fabs(inputSampleL) > offthreshold)
				{
					if (gaterollerL < ZeroCrossL) gaterollerL = ZeroCrossL;
					else gaterollerL -= release;
					//always trigger if gate is over offthreshold, otherwise close anyway
				}
				else gaterollerL -= release;
			}

			if (gateR == 0.0)
			{
				//if gate is totally silent
				if (fabs(inputSampleR) > onthreshold)
				{
					if (gaterollerR == 0.0) gaterollerR = ZeroCrossR;
					else gaterollerR -= release;
					// trigger from total silence only- if we're active then signal must clear offthreshold
				}
				else gaterollerR -= release;
			}
			else {
				//gate is not silent but closing
				if (fabs(inputSampleR) > offthreshold)
				{
					if (gaterollerR < ZeroCrossR) gaterollerR = ZeroCrossR;
					else gaterollerR -= release;
					//always trigger if gate is over offthreshold, otherwise close anyway
				}
				else gaterollerR -= release;
			}

			if (gaterollerL < 0.0) gaterollerL = 0.0;
			if (gaterollerR < 0.0) gaterollerR = 0.0;

			if (gaterollerL < 1.0)
			{
				gateL = gaterollerL;
				double bridgerectifier = 1 - cos(fabs(inputSampleL));
				if (inputSampleL > 0) inputSampleL = (inputSampleL * gateL) + (bridgerectifier * (1.0 - gateL));
				else inputSampleL = (inputSampleL * gateL) - (bridgerectifier * (1.0 - gateL));
				if (gateL == 0.0) inputSampleL = 0.0;
			}
			else gateL = 1.0;

			if (gaterollerR < 1.0)
			{
				gateR = gaterollerR;
				double bridgerectifier = 1 - cos(fabs(inputSampleR));
				if (inputSampleR > 0) inputSampleR = (inputSampleR * gateR) + (bridgerectifier * (1.0 - gateR));
				else inputSampleR = (inputSampleR * gateR) - (bridgerectifier * (1.0 - gateR));
				if (gateR == 0.0) inputSampleR = 0.0;
			}
			else gateR = 1.0;
			//end Gate

			*in1 = inputSampleL;
			*in2 = inputSampleR;

			in1++;
			in2++;
		}
	}


	template <typename SampleType>
	void lunchboxProcessor::processInflator(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames)
	{
		SampleType* in1 = (SampleType*)inputs[0];
		SampleType* in2 = (SampleType*)inputs[1];

		Vst::Sample64 curvepct = 0.5 - 0.5;
		Vst::Sample64 curveA = 1.5 + curvepct;			// 1 + (curve + 50) / 100
		Vst::Sample64 curveB = -(curvepct + curvepct);	// - curve / 50
		Vst::Sample64 curveC = curvepct - 0.5;			// (curve - 50) / 100
		Vst::Sample64 curveD = 0.0625 - curvepct * 0.25 + (curvepct * curvepct) * 0.25;	// 1 / 16 - curve / 400 + curve ^ 2 / (4 * 10 ^ 4)

		Vst::Sample64 s1_L, s1_R;
		Vst::Sample64 s2_L, s2_R;
		Vst::Sample64 s3_L, s3_R;
		Vst::Sample64 s4_L, s4_R;

		Vst::Sample64 signL;
		Vst::Sample64 signR;

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;
			Vst::Sample64 drySampleL = inputSampleL;
			Vst::Sample64 drySampleR = inputSampleR;

			if (bParamSafe) {
				if (inputSampleL > 1.0)
					inputSampleL = 1.0;
				else if (inputSampleL < -1.0)
					inputSampleL = -1.0;

				if (inputSampleR > 1.0)
					inputSampleR = 1.0;
				else if (inputSampleR < -1.0)
					inputSampleR = -1.0;
			}

			if (inputSampleL > 0.0)
				signL = 1.0;
			else
				signL = -1.0;

			if (inputSampleR > 0.0)
				signR = 1.0;
			else
				signR = -1.0;

			s1_L = fabs(inputSampleL);
			s2_L = s1_L * s1_L;
			s3_L = s2_L * s1_L;
			s4_L = s2_L * s2_L;

			s1_R = fabs(inputSampleR);
			s2_R = s1_R * s1_R;
			s3_R = s2_R * s1_R;
			s4_R = s2_R * s2_R;

			if (s1_L >= 2.0)
				inputSampleL = 0.0;
			else if (s1_L > 1.0)
				inputSampleL = (2.0 * s1_L) - s2_L;
			else
				inputSampleL = (curveA * s1_L) + (curveB * s2_L) + (curveC * s3_L) - (curveD * (s2_L - (2.0 * s3_L) + s4_L));

			if (s1_R >= 2.0)
				inputSampleR = 0.0;
			else if (s1_R > 1.0)
				inputSampleR = (2.0 * s1_R) - s2_R;
			else
				inputSampleR = (curveA * s1_R) + (curveB * s2_R) + (curveC * s3_R) - (curveD * (s2_R - (2.0 * s3_R) + s4_R));

			inputSampleL *= signL;
			inputSampleR *= signR;

			if (fParamInflate != 1.0) {
				inputSampleL = (drySampleL * (1.0 - fParamInflate)) + (inputSampleL * fParamInflate);
				inputSampleR = (drySampleR * (1.0 - fParamInflate)) + (inputSampleR * fParamInflate);
			}

			if (bParamSafe) {
				if (inputSampleL > 1.0)
					inputSampleL = 1.0;
				else if (inputSampleL < -1.0)
					inputSampleL = -1.0;

				if (inputSampleR > 1.0)
					inputSampleR = 1.0;
				else if (inputSampleR < -1.0)
					inputSampleR = -1.0;
			}

			*in1 = inputSampleL;
			*in2 = inputSampleR;

			in1++;
			in2++;
		}
		return;
	}

	template <typename SampleType>
	void lunchboxProcessor::processInput(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames)
	{
		SampleType* in1 = (SampleType*)inputs[0];
		SampleType* in2 = (SampleType*)inputs[1];
		Vst::Sample64 In_db = norm_to_gain((Vst::Sample64)fParamInput);

		Vst::Sample64 tmpIn = 0.0; /*/ VuPPM /*/

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;
			inputSampleL *= In_db;
			inputSampleR *= In_db;
			if (inputSampleL > tmpIn) { tmpIn = inputSampleL; }
			if (inputSampleR > tmpIn) { tmpIn = inputSampleR; }
			if (fabs(inputSampleL) < 1.18e-23) inputSampleL = fpdL * 1.18e-17;
			if (fabs(inputSampleR) < 1.18e-23) inputSampleR = fpdR * 1.18e-17;
			*in1 = inputSampleL;
			*in2 = inputSampleR;
			in1++;
			in2++;
		}

		/*/ VuPPM /*/
		fParamInVuPPM = VuPPMconvert(tmpIn, -60.0, 0.0, -18.0);

		return;
	}

	template <typename SampleType>
	void lunchboxProcessor::processOutput(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames, int32 precision)
	{
		SampleType* in1 = (SampleType*)inputs[0];
		SampleType* in2 = (SampleType*)inputs[1];
		Vst::Sample64 Out_db = norm_to_gain((Vst::Sample64)fParamOutput);

		Vst::Sample64 tmpOut = 0.0; /*/ VuPPM /*/

		while (--sampleFrames >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;
			inputSampleL *= Out_db;
			inputSampleR *= Out_db;
			if (inputSampleL > tmpOut) { tmpOut = inputSampleL; }
			if (inputSampleR > tmpOut) { tmpOut = inputSampleR; }
			if (precision == 0) {
				//begin 32 bit stereo floating point dither
				int expon; frexpf((float)inputSampleL, &expon);
				fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
				inputSampleL += ((double(fpdL) - uint32_t(0x7fffffff)) * 5.5e-36l * pow(2, expon + 62));
				frexpf((float)inputSampleR, &expon);
				fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
				inputSampleR += ((double(fpdR) - uint32_t(0x7fffffff)) * 5.5e-36l * pow(2, expon + 62));
				//end 32 bit stereo floating point dither
			}
			else {
				//begin 64 bit stereo floating point dither
				//int expon; frexp((double)inputSampleL, &expon);
				fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
				//inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
				//frexp((double)inputSampleR, &expon);
				fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
				//inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
				//end 64 bit stereo floating point dither
			}
			*in1 = inputSampleL;
			*in2 = inputSampleR;
			in1++;
			in2++;
		}

		/*/ VuPPM /*/
		fParamOutVuPPM = VuPPMconvert(tmpOut, -60.0, 0.0, -18.0);

		return;
	}

	template <typename SampleType>
	void lunchboxProcessor::processBypass(SampleType** inputs, Vst::Sample64 getSampleRate, int32 sampleFrames) 
	{
		SampleType* in1 = (SampleType*)inputs[0];
		SampleType* in2 = (SampleType*)inputs[1];

		Vst::Sample64 tmpIn = 0.0; /*/ VuPPM /*/

		int32 samples = sampleFrames;

		while (--samples >= 0)
		{
			Vst::Sample64 inputSampleL = *in1;
			Vst::Sample64 inputSampleR = *in2;
			if (inputSampleL > tmpIn) { tmpIn = inputSampleL; }
			if (inputSampleR > tmpIn) { tmpIn = inputSampleR; }
			in1++;
			in2++;
		}

		/*/ VuPPM /*/
		fParamInVuPPM = VuPPMconvert(tmpIn, -60.0, 0.0, -18.0);
		fParamOutVuPPM = fParamInVuPPM;

		return;
	}

	inline void lunchboxProcessor::setCoeffs(double Fs)
	{
		while (fpdL < 16386) fpdL = rand() * UINT32_MAX;
		while (fpdR < 16386) fpdR = rand() * UINT32_MAX;

		double Fc, K, Q, norm;

		Q = 0.51763809;
		// 160Hz

		Fc = 10.0;
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (1.0 + K / Q + K * K);
		z_10[0] = K / Q * norm;
		z_10[1] = 0.0;
		z_10[2] = -z_10[0];
		p_10[0] = 1.0;
		p_10[1] = 2.0 * (K * K - 1.0) * norm;
		p_10[2] = (1.0 - K / Q + K * K) * norm;

		Fc = 40.0;
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (1.0 + K / Q + K * K);
		z_40[0] = K / Q * norm;
		z_40[1] = 0.0;
		z_40[2] = -z_40[0];
		p_40[0] = 1.0;
		p_40[1] = 2.0 * (K * K - 1.0) * norm;
		p_40[2] = (1.0 - K / Q + K * K) * norm;

		Fc = 160.0;
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (1.0 + K / Q + K * K);
		z_160[0] = K / Q * norm;
		z_160[1] = 0.0;
		z_160[2] = -z_160[0];
		p_160[0] = 1.0;
		p_160[1] = 2.0 * (K * K - 1.0) * norm;
		p_160[2] = (1.0 - K / Q + K * K) * norm;

		// 640Hz
		Fc = 640.0;
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (1.0 + K / Q + K * K);
		z_640[0] = K / Q * norm;
		z_640[1] = 0.0;
		z_640[2] = -z_640[0];
		p_640[0] = 1.0;
		p_640[1] = 2.0 * (K * K - 1.0) * norm;
		p_640[2] = (1.0 - K / Q + K * K) * norm;

		// 1200Hz
		Fc = 1200.0;
		Q_1k2 = 1.5;
		K_1k2 = tan(M_PI * Fc / Fs);
		K_1k2_2 = K_1k2 * K_1k2;

		// 2500Hz
		Fc = 1200.0; // YES
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (K + 1.0);
		z_2k5[0] = norm;
		z_2k5[1] = -norm;
		z_2k5[2] = 0.0;
		p_2k5[0] = 1.0;
		p_2k5[1] = (K - 1) * norm;
		p_2k5[2] = 0.0;

		// 20000Hz
		Fc = 10500.0; //YES
		K = tan(M_PI * Fc / Fs);
		norm = 1.0 / (K + 1.0);
		z_20k[0] = norm;
		z_20k[1] = -norm;
		z_20k[2] = 0.0;
		p_20k[0] = 1.0;
		p_20k[1] = (K - 1) * norm;
		p_20k[2] = 0.0;

		for (int i = 0; i < 3; i++) {
			z_10[i] *= 5.623413;
			z_40[i] *= 5.623413;
			z_160[i] *= 5.623413;
			z_640[i] *= 5.623413;
			z_2k5[i] *= 7.943282;
			z_20k[i] *= 7.943282;
		}
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
	{
		//--- called before any processing ----
		setCoeffs(newSetup.sampleRate);
		return AudioEffect::setupProcessing(newSetup);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::canProcessSampleSize(int32 symbolicSampleSize)
	{
		// by default kSample32 is supported
		if (symbolicSampleSize == Vst::kSample32)
			return kResultTrue;

		if (symbolicSampleSize == Vst::kSample64)
			return kResultTrue;

		return kResultFalse;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::setState(IBStream* state)
	{
		// called when we load a preset, the model has to be reloaded
		IBStreamer streamer(state, kLittleEndian);

		float savedInput = 0.f;
		if (streamer.readFloat(savedInput) == false)
			return kResultFalse;
		fParamInput= savedInput;

		float savedOutput = 0.f;
		if (streamer.readFloat(savedOutput) == false)
			return kResultFalse;
		fParamOutput= savedOutput;

		float savedDrive = 0.f;
		if (streamer.readFloat(savedDrive) == false)
			return kResultFalse;
		fParamDrive = savedDrive;

		float savedAir = 0.f;
		if (streamer.readFloat(savedAir) == false)
			return kResultFalse;
		fParamAir = savedAir;

		float savedHigh = 0.f;
		if (streamer.readFloat(savedHigh) == false)
			return kResultFalse;
		fParamHigh = savedHigh;

		float savedFocus = 0.f;
		if (streamer.readFloat(savedFocus) == false)
			return kResultFalse;
		fParamFocus = savedFocus;

		float savedBody = 0.f;
		if (streamer.readFloat(savedBody) == false)
			return kResultFalse;
		fParamBody = savedBody;

		float savedLow = 0.f;
		if (streamer.readFloat(savedLow) == false)
			return kResultFalse;
		fParamLow = savedLow;


		float savedIntensity = 0.f;
		if (streamer.readFloat(savedIntensity) == false)
			return kResultFalse;
		fParamIntensity= savedIntensity;

		float savedSharpness = 0.f;
		if (streamer.readFloat(savedSharpness) == false)
			return kResultFalse;
		fParamSharpness= savedSharpness;
		
		float savedDepth = 0.f;
		if (streamer.readFloat(savedDepth) == false)
			return kResultFalse;
		fParamDepth= savedDepth;

		float savedComp = 0.f;
		if (streamer.readFloat(savedComp) == false)
			return kResultFalse;
		fParamComp= savedComp;

		float savedSpeed = 0.f;
		if (streamer.readFloat(savedSpeed) == false)
			return kResultFalse;
		fParamSpeed= savedSpeed;


		float savedGate = 0.f;
		if (streamer.readFloat(savedGate) == false)
			return kResultFalse;
		fParamGate= savedGate;

		float savedInflate = 0.f;
		if (streamer.readFloat(savedInflate) == false)
			return kResultFalse;
		fParamInflate= savedInflate;

		int32 savedLowcut = 0;
		if (streamer.readInt32(savedLowcut) == false)
			return kResultFalse;
		bParamLowcut = savedLowcut;

		int32 savedListen = 0;
		if (streamer.readInt32(savedListen) == false)
			return kResultFalse;
		bParamListen = savedListen;

		int32 savedAttack = 0;
		if (streamer.readInt32(savedAttack) == false)
			return kResultFalse;
		bParamAttack = savedAttack;

		int32 savedSafe = 0;
		if (streamer.readInt32(savedSafe) == false)
			return kResultFalse;
		bParamSafe = savedSafe;

		int32 savedBypass = 0;
		if (streamer.readInt32(savedBypass) == false)
			return kResultFalse;
		bParamBypass = savedBypass;




		if (Vst::Helpers::isProjectState(state) == kResultTrue)
		{
			// we are in project loading context...

			// Example of using the IStreamAttributes interface
			FUnknownPtr<Vst::IStreamAttributes> stream(state);
			if (stream)
			{
				if (Vst::IAttributeList* list = stream->getAttributes())
				{
					// get the full file path of this state
					Vst::TChar fullPath[1024];
					memset(fullPath, 0, 1024 * sizeof(Vst::TChar));
					if (list->getString(Vst::PresetAttributes::kFilePathStringType, fullPath,
						1024 * sizeof(Vst::TChar)) == kResultTrue)
					{
						// here we have the full path ...
					}
				}
			}
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API lunchboxProcessor::getState(IBStream* state)
	{
		// here we need to save the model
		IBStreamer streamer(state, kLittleEndian);

		streamer.writeFloat(fParamInput);
		streamer.writeFloat(fParamOutput);

		streamer.writeFloat(fParamDrive);
		streamer.writeFloat(fParamAir);
		streamer.writeFloat(fParamHigh);
		streamer.writeFloat(fParamFocus);
		streamer.writeFloat(fParamBody);
		streamer.writeFloat(fParamLow);

		streamer.writeFloat(fParamIntensity);
		streamer.writeFloat(fParamSharpness);
		streamer.writeFloat(fParamDepth);
		streamer.writeFloat(fParamComp);
		streamer.writeFloat(fParamSpeed);
		streamer.writeFloat(fParamGate);
		streamer.writeFloat(fParamInflate);

		streamer.writeInt32(bParamLowcut ? 1 : 0);
		streamer.writeInt32(bParamListen ? 1 : 0);
		streamer.writeInt32(bParamAttack ? 1 : 0);
		streamer.writeInt32(bParamSafe ? 1 : 0);
		streamer.writeInt32(bParamBypass ? 1 : 0);


		return kResultOk;
	}

	//------------------------------------------------------------------------
} // namespace yg331
