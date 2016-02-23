//-----------------------------------------------------------------------------
//@file  
//	filter.cpp
//
//@author
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//@brief 
//	Implementation of the filter class.
//
//  Example user module to show how to process audio buffers.
//
//@historic 
//	2015/02/23
//    first release for Hollyhock CPP SDK 6.04.001
//
//@IMPORTANT
//	This file is part of the Usine Hollyhock CPP SDK
//
//  Please, report bugs and patch to Usine forum :
//  http://www.sensomusic.com/wiki2/doku.php?id=hollyhock:bugsreport 
//
// All dependencies are under there own licence.
//
//@LICENCE
// Copyright (C) 2013, 2014, 2015 Sensomusic
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of 
// this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software 
// is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
//     copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "filter.h"
// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "Oli_Ladder Filter";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "Oli_Ladder Filter";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.4";
//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule(void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new filter();
}

// destroy
void DestroyModule(void* pModule)
{
	// cast is important to call the good destructor
	delete ((filter*)pModule);
}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
filter::filter()
	: drive(1)//, cutoff(1)
{
	// audio smooth
	m_tevtSmoothCurrentDrive = NULL;
	//m_tevtSmoothCurrentCutoff = NULL;
}

// destructor
filter::~filter()
{
	if (m_tevtSmoothCurrentDrive != NULL)
		sdkDestroyEvt(m_tevtSmoothCurrentDrive);
	//if (m_tevtSmoothCurrentCutoff != NULL)
		//sdkDestroyEvt(m_tevtSmoothCurrentCutoff);
}

// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void filter::onGetModuleInfo(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = MODULE_NAME;
	pModuleInfo->Description = MODULE_DESC;
	pModuleInfo->Version = MODULE_VERSION;
	pModuleInfo->ModuleType = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clAudioModuleColor) + 0x102011;

	// query for multi-channels
	if (pMasterInfo != nullptr)
	{
		pModuleInfo->QueryString = sdkGetAudioQueryTitle();
		pModuleInfo->QueryListValues = sdkGetAudioQueryChannelList();
		pModuleInfo->QueryDefaultIdx = 1;
	}
}

//-----------------------------------------------------------------------------
// query system and init methodes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Get total parameters number of the module
int filter::onGetNumberOfParams(int queryIndex)
{
	int result = 0;
	this->queryIndex = queryIndex;
	numOfAudiotInsOuts = sdkGetAudioQueryToNbChannels(queryIndex);

	// we want 1 in 1 out per channels
	result = (numOfAudiotInsOuts * 2) + numOfParamAfterAudiotInOut;

	return result;
}

//-----------------------------------------------------------------------------
// Called after the query popup
void filter::onAfterQuery(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	sdkCreateEvt(m_tevtSmoothCurrentDrive, pMasterInfo->BlocSize);
	//sdkCreateEvt(m_tevtSmoothCurrentCutoff, pMasterInfo->BlocSize);
}


//-----------------------------------------------------------------------------
// initialisation
void filter::onInitModule(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	/*
	"factor" depends on sampling rate and the low frequency point.
	Do not set "factor" to a fixed value(e.g. 0.99) if you don't know the sample rate.
	Instead set R to:
	(-3dB @ 40Hz) : factor = 1 - (250 / samplerate)
	(-3dB @ 30Hz) : factor = 1 - (190 / samplerate)
	(-3dB @ 20Hz) : factor = 1 - (126 / samplerate)
	*/
	factor = 1.0f - (30.0f / sdkGetSampleRate()); //environ 5Hz ?
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		audioBuffer0[i] = 0;
		audioBuffer1[i] = 0;
		audioBuffer2[i] = 0;
		audioBuffer3[i] = 0;
		audioBuffer4[i] = 0;
		xm1[i] = 0;
		ym1[i] = 0;
		y[i] = 0;
		}
		inv_samplerate = 1 / sdkGetSampleRate();
		resonanceRaw = 0;
		driveRaw = 0;
	
	t1 = 0;
	t2 = 0;
}

//----------------------------------------------------------------------------
// parameters and process
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void filter::onGetParamInfo(int ParamIndex, TParamInfo* pParamInfo)
{
	// audioInputs
	if (ParamIndex < numOfAudiotInsOuts)
	{
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = sdkGetAudioQueryChannelNames("in ", ParamIndex + 1, queryIndex);
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->ReadOnly = FALSE;

		if (ParamIndex == 0)
		{
			pParamInfo->IsSeparator = TRUE;
			pParamInfo->SeparatorCaption = "audio in";
		}
	}
	// audioOutputs
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = sdkGetAudioQueryChannelNames("out ", ParamIndex - numOfAudiotInsOuts + 1, queryIndex);
		pParamInfo->IsInput = FALSE;
		pParamInfo->IsOutput = TRUE;
		pParamInfo->ReadOnly = TRUE;

		if (ParamIndex == numOfAudiotInsOuts)
		{
			pParamInfo->IsSeparator = TRUE;
			pParamInfo->SeparatorCaption = "audio out";
		}
	}

	// filter cutoff
	else if (ParamIndex == numOfAudiotInsOuts * 2) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "cutoff";
		pParamInfo->MinValue = 10.0f;
		pParamInfo->MaxValue = 20000.0f;
		pParamInfo->DefaultValue = 20000.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLog;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}
	// resonance
	else if (ParamIndex == (1 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "resonance";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		//pParamInfo->Symbol = "coeff";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}

	else if (ParamIndex == (2 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "drive";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLog;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}

	//prévoir entrée audio pour filter FM
	else if (ParamIndex == (3 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = "FM in";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->ReadOnly = FALSE;
	}
	//prevoir entrée 0-1 pour Filter FM amount (growl)
	else if (ParamIndex == (4 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "FM amount";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}

	else if (ParamIndex == (5 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptListBox;
		pParamInfo->Caption = "feedback topology";
		pParamInfo->ListBoxStrings = "\"P4\",\"P3\",\"P2\",\"P1\",\"Bending\",\"Rectifier\",\"D-Rectifier\",\"bit mangler\",\"decimate\",\"overdriven\",\"filter topology\"";
		pParamInfo->DefaultValue = 0;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		//pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (6 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptListBox;
		pParamInfo->Caption = "Filter mode";
		pParamInfo->ListBoxStrings = "\"LP4\",\"LP3\",\"LP2\",\"LP1\",\"HP4\",\"HP3\",\"HP2\",\"HP1\",\"BP2\"";
		pParamInfo->DefaultValue = 0;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		//pParamInfo->CallBackType = ctImmediate;
	}

}

//-----------------------------------------------------------------------------
// set the parameters events address
void filter::onSetEventAddress(int ParamIndex, UsineEventPtr pEvent)
{
	// audioInputs
	if (ParamIndex < numOfAudiotInsOuts)
	{
		audioInputs[ParamIndex] = pEvent;
	}
	// audioOutputs
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 2))
	{
		audioOutputs[ParamIndex - numOfAudiotInsOuts] = pEvent;
	}
	// cutoff
	else if (ParamIndex == numOfAudiotInsOuts * 2)
	{
		m_cutoff = pEvent;
	}
	// resonance
	else if (ParamIndex == (1 + numOfAudiotInsOuts * 2))
	{
		m_resonance = pEvent;
	}
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 2))
	{
		m_drive = pEvent;
	}
	else if (ParamIndex == (3 + numOfAudiotInsOuts * 2))
	{
		audioFMinput = pEvent;
	}
	else if (ParamIndex == (4 + numOfAudiotInsOuts * 2))
	{
		m_growl = pEvent;
	}
	else if (ParamIndex == (5 + numOfAudiotInsOuts * 2))
	{
		m_feedback = pEvent;
	}
	else if (ParamIndex == (6 + numOfAudiotInsOuts * 2))
	{
		m_type = pEvent;
	}

}

//-----------------------------------------------------------------------------
// Parameters callback
void filter::onCallBack(UsineMessage *Message)
{
	// filter only message specific to this user module
	if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
	{
		// Message->wParam is equal to ParamIndex
		if ((Message->wParam == (numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			cutoff = sdkGetEvtData(m_cutoff);
			cutoffFinal = cutoff * inv_samplerate;
		}
		else if ((Message->wParam == (1 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			resonanceRaw = sdkGetEvtData(m_resonance);
		}
		else if ((Message->wParam == (2 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			driveRaw = sdkGetEvtData(m_drive);
		}
		else if ((Message->wParam == (4 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			growlRaw = sdkGetEvtData(m_growl);
			if (growlRaw > 0) {
				growlRaw = powf(1000, growlRaw)*0.0001f;
			}
			else { growlRaw = 0; }
		}
		else if ((Message->wParam == (5 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			feedback = sdkGetEvtData(m_feedback);
		}
		else if ((Message->wParam == (6 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{			
			type = sdkGetEvtData(m_type);
		}
	}
}

void filter::onProcess() {
	//sdkSmoothEvent(m_smoothOldCutoff, m_tevtSmoothCurrentCutoff, cutoff, 0.1f);
	sdkSmoothPrecision(cutoff, m_cutoff, 0.1f);
	drive = driveRaw + 1.0f;
	sdkSmoothEvent(m_smoothOldDrive, m_tevtSmoothCurrentDrive, drive, 0.999f);

	if (growlRaw <= 0.000001f) {
		cutoffFinal = cutoff * inv_samplerate;
		computeCoeff();
		//sdkTracePrecision(f);
	}
	else {}

	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		sdkCopyEvt(audioInputs[i], audioOutputs[i]);
		sdkMultEvt2Audio(m_tevtSmoothCurrentDrive, audioInputs[i]);
		//filter

		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			if (growlRaw > 0.000001f) {
				//cutoffFinal = cutoff + growl * sdkGetEvtArrayData(audioFMinput, j) * cutoff / 10000.0f;
				growl = growlRaw * sdkGetEvtArrayData(audioFMinput, j);
				cutoffFinal = cutoff * inv_samplerate;
				cutoffFinal += growl;
				cutoffFinal = cutoffFinal / (1 + abs(cutoffFinal));//soft clipping
				if (cutoffFinal < 0) {cutoffFinal = 0;}
				computeCoeff();
			}
			else {}
			/*
			// algorithm
			in -= q * b4;                          //feedback
			t1 = b1;  b1 = (in + b0) * p - b1 * f;
			t2 = b2;  b2 = (b1 + t1) * p - b2 * f;
			t1 = b3;  b3 = (b2 + t2) * p - b3 * f;
			b4 = (b3 + t1) * p - b4 * f;
			b4 = b4 - b4 * b4 * b4 * 0.166667f;    //clipping
			b0 = in;
			*/
			
			int b_oversample = 0;
			float b_inSH = sdkGetEvtArrayData(audioInputs[i], j); // before the while statement.
			while (b_oversample < 2) {
				//4x oversampling

			switch (feedback) {

			case 0 :
				in = resonance * audioBuffer4[i] * 1.2f;
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 1.7f;
				in = b_inSH - in; //what goes in the filter
				//protection
				if (in > 1.0f) { in = 1.0f; }
				else if (in < -1.0f) { in = -1.0f; }
				else {}
				break;

			case 1:
				in = resonance * audioBuffer3[i];
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 2.2f;
				in = b_inSH - in; //what goes in the filter
				break;

			case 2:
				in = resonance * audioBuffer2[i];
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 3.5f;
				in = b_inSH - in; //what goes in the filter
				break;

			case 3:
				in = resonance * audioBuffer1[i];
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 4.7f;
				in = b_inSH - in; //what goes in the filter
				break;

			case 4 : //bending
				in = resonance * y[i];
				in = (in + in) / (1 + abs(in));//soft clipping du feedback
				in = b_inSH + in; //what goes in the filter bend
				break;

			case 5: //rectifier
				in = resonance * resonanceBuffer[i];
				if (in > 0) {
					in -= sqrtdivPI;
					in = (in+in) / (1 + abs(in));//soft clipping du feedback
					in = b_inSH - in; //what goes in the filter
				}
				else {
					in = b_inSH;
				}
				break;

			case 6: //double rectifier
				in = resonance * resonanceBuffer[i];
					in  = abs(in);
					in = in - sqrtdivPI - sqrtdivPI;
					in = (in+in) / (1 + abs(in));//soft clipping du feedback
					in = b_inSH - in; //what goes in the filter
				break;

			case 7: //bit mangler
				in = resonance * resonanceBuffer[i];
				in = truncf(in * 16);
				mangled = in;
				mangled = mangled & count;
				count++;
				in = mangled;
				in = in / 16;
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 2.2f;
				in = b_inSH - in; //what goes in the filter
				break;

			case 8: //decimate
				in = resonance * resonanceBuffer[i];
				in = truncf(in * 4);
				in = in / 4;
				in = in / (1 + abs(in));//soft clipping du feedback
				in *= 2.2f;
				in = b_inSH - in; //what goes in the filter
				break;
			case 9: //overdriven
				//in = 5 * resonanceBuffer[i];
				in = resonanceBuffer[i];

				//in = resonance * in;
				//in = in / (1 + fabsf(in));
				in = expf(in*(a + G)) - expf(in*(b - G));
				in = in / (expf(in*G) + expf(in * (-G)));
				in = resonance * in;
				in *= (cutoffFinal - 1.55f);
				
				in = b_inSH + in; //what goes in the filter 
				
			break;
			case 10: //follow filter topology
				in = resonance * y[i] * 1.2f;
				//in = (in + in + in) / (1 + abs(in));//soft clipping du feedback
				in = b_inSH - in; //what goes in the filter 
				break;
			default : 
				break;
			    }
			
			//
				t1 = audioBuffer1[i];
				audioBuffer1[i] = (in + audioBuffer0[i])*p - audioBuffer1[i] * f;
				/*
				if (audioBuffer1[i] > 1.0f) { audioBuffer1[i] = 1.0f; } 
				else if (audioBuffer1[i] < -1.0f) { audioBuffer1[i] = -1.0f; }
				else {}
				*/
				t2 = audioBuffer2[i];
				audioBuffer2[i] = (t1 + audioBuffer1[i])*p - audioBuffer2[i] * f;
				/*
				if (audioBuffer2[i] > 1.0f) { audioBuffer2[i] = 1.0f; }
				else if (audioBuffer2[i] < -1.0f) { audioBuffer2[i] = -1.0f; }
				else {}
				*/
				t1 = audioBuffer3[i];
				audioBuffer3[i] = (t2 + audioBuffer2[i])*p - audioBuffer3[i] * f;
				/*
				if (audioBuffer3[i] > 1.0f) { audioBuffer3[i] = 1.0f; }
				else if (audioBuffer3[i] < -1.0f) { audioBuffer3[i] = -1.0f; }
				else {}
				*/
				audioBuffer4[i] = (t1 + audioBuffer3[i])*p - audioBuffer4[i] * f;
				/*
				if (audioBuffer4[i] > 1.0f) { audioBuffer4[i] = 1.0f; }
				else if (audioBuffer4[i] < -1.0f) { audioBuffer4[i] = -1.0f; }
				else {}
				*/
				audioBuffer0[i] = in;
				//fin du filtre
			//			
			
			switch (type) {
			case 0:  //LP4
			    //Clipper band limited sigmoid
				audioBuffer4[i] = k*(audioBuffer4[i] - (audioBuffer4[i] * audioBuffer4[i] * audioBuffer4[i]) * 0.1666666667f);
				y[i] = audioBuffer4[i];
				resonanceBuffer[i] = audioBuffer4[i];
				
				break;
			case 1:  //LP3
				//Clipper band limited sigmoid
				audioBuffer3[i] = k*(audioBuffer3[i] - (audioBuffer3[i] * audioBuffer3[i] * audioBuffer3[i]) * 0.1666666667f);
				y[i] = audioBuffer3[i];
				resonanceBuffer[i] = audioBuffer3[i];
				break;
			case 2:  //LP2
				//Clipper band limited sigmoid
				audioBuffer2[i] = k*(audioBuffer2[i] - (audioBuffer2[i] * audioBuffer2[i] * audioBuffer2[i]) * 0.1666666667f);
				y[i] = audioBuffer2[i];
				resonanceBuffer[i] = audioBuffer3[i];
				break;
			case 3:  //LP1
					 //Clipper band limited sigmoid
				audioBuffer1[i] = k*(audioBuffer1[i] - (audioBuffer1[i] * audioBuffer1[i] * audioBuffer1[i]) * 0.1666666667f);
				y[i] = audioBuffer1[i];
				break;
			case 4: //HP4
					//Clipper band limited sigmoid
				audioBuffer4[i] = k*(audioBuffer4[i] - (audioBuffer4[i] * audioBuffer4[i] * audioBuffer4[i]) * 0.1666666667f);
				y[i] = in - audioBuffer4[i];
				resonanceBuffer[i] = y[i];
				break;
			case 5: //HP3
				audioBuffer3[i] = k*(audioBuffer3[i] - (audioBuffer3[i] * audioBuffer3[i] * audioBuffer3[i]) * 0.1666666667f);
				y[i] = in - audioBuffer3[i];
				resonanceBuffer[i] = y[i];
				break;
			case 6: //HP2
				audioBuffer2[i] = k*(audioBuffer2[i] - (audioBuffer2[i] * audioBuffer2[i] * audioBuffer2[i]) * 0.1666666667f);
				y[i] = in - audioBuffer2[i];
				resonanceBuffer[i] = y[i];
				break;
			case 7: //HP1
				audioBuffer1[i] = k*(audioBuffer1[i] - (audioBuffer1[i] * audioBuffer1[i] * audioBuffer1[i]) * 0.1666666667f);
				y[i] = in - audioBuffer1[i];
				resonanceBuffer[i] = y[i];
				break;
			case 8: //BP
				audioBuffer4[i] = k*(audioBuffer4[i] - (audioBuffer4[i] * audioBuffer4[i] * audioBuffer4[i]) * 0.1666666667f);
				y[i] = 3.0f*(audioBuffer3[i] - audioBuffer4[i]);
				break;
			}
			b_oversample++;
			}

			if (feedback >= 4) {
				//DC blocker 
				y[i] = audioBuffer4[i] - xm1[i] + factor * ym1[i];
				xm1[i] = audioBuffer4[i];
				ym1[i] = y[i];
			}
			//sdkTracePrecision(scale);
			sdkSetEvtArrayData(audioOutputs[i], j, scale * y[i]);
		}
	}
}


void filter::computeCoeff() {
	k = 1.0f - cutoffFinal;
	p = cutoffFinal + 0.8f * cutoffFinal * k;
	f = p + p - 1.0f;
	resonance = resonanceRaw * (1.0f + 0.4f * k * (1.0f - k + 5.6f * k * k)); //(1.0f + 0.5f * k * (1.0f - k + 5.6f * k * k));
	//sdkTracePrecision(resonance);
	//scale = exp2f((1 - p)*1.386249);
	//sdkTracePrecision(cutoffFinal);
	if (type <= 3) { // its an LP
		scale = map(cutoffFinal, 0.01f, 0.0002f, 0.7f, 3.0f);
	}
	else {
		scale = 0.5f;

	}
	//sdkTracePrecision(scale);
	//scale *= scale;
}


float filter::map(float x, float in_min, float in_max, float out_min, float out_max)
{
float tempVal = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
if (tempVal < out_min) { tempVal = out_min; }
if (tempVal > out_max) { tempVal = out_max; }
	return tempVal;
}


