//-----------------------------------------------------------------------------
//@file  
//	filterbutterworth.cpp
//
//@author
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//@brief 
//	Implementation of the filterbutterworth class.
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
#include "butterworth.h"
// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "butterworth HP 2x";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "butterworth HP 2x";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.3";
//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule (void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new filterbutterworth();
}

// destroy
void DestroyModule(void* pModule) 
{
	// cast is important to call the good destructor
	delete ((filterbutterworth*)pModule);
}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
filterbutterworth::filterbutterworth()
{

}

// destructor
filterbutterworth::~filterbutterworth()
{

}

// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void filterbutterworth::onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = MODULE_NAME;
	pModuleInfo->Description = MODULE_DESC;
	pModuleInfo->Version = MODULE_VERSION;
	pModuleInfo->ModuleType         = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clAudioModuleColor) + 0x102011;
    
	// query for multi-channels
	if (pMasterInfo != nullptr)
    {
	    pModuleInfo->QueryString		= sdkGetAudioQueryTitle();
	    pModuleInfo->QueryListValues	= sdkGetAudioQueryChannelList();
	    pModuleInfo->QueryDefaultIdx	= 1;
    }
}

//-----------------------------------------------------------------------------
// query system and init methodes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Get total parameters number of the module
int filterbutterworth::onGetNumberOfParams (int queryIndex)
{
	int result = 0;
    this->queryIndex = queryIndex;
    numOfAudiotInsOuts = sdkGetAudioQueryToNbChannels (queryIndex);

    // we want 1 in 1 out per channels
	result = (numOfAudiotInsOuts * 2) + numOfParamAfterAudiotInOut;

    return result;
}

//-----------------------------------------------------------------------------
// Called after the query popup
void filterbutterworth::onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	//
}


//-----------------------------------------------------------------------------
// initialisation
void filterbutterworth::onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) 
{
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{

			audioBufferN2[i] = 0;
			audioBufferN1[i] = 0;
			inv_samplerate = 1/sdkGetSampleRate();
			resonanceRaw = 0;
			driveRaw = 0;
	}
}

//----------------------------------------------------------------------------
// parameters and process
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void filterbutterworth::onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo)
{	
    // audioInputs
    if (ParamIndex < numOfAudiotInsOuts)
    {
		pParamInfo->ParamType		= ptAudio;
		pParamInfo->Caption			= sdkGetAudioQueryChannelNames ( "in ", ParamIndex + 1, queryIndex);
		pParamInfo->IsInput			= TRUE;
		pParamInfo->IsOutput		= FALSE;
		pParamInfo->ReadOnly		= FALSE;

        if (ParamIndex == 0)
        {
            pParamInfo->IsSeparator     = TRUE;
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
    else if (ParamIndex == numOfAudiotInsOuts*2) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "cutoff";
		pParamInfo->MinValue = 10.0f;
		pParamInfo->MaxValue = 20000.0f;
		pParamInfo->DefaultValue = 10.0f;
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
		//pParamInfo->MinValue = 0.001f;
		//pParamInfo->MaxValue = 1.414213f;
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

	//entrée audio pour filter FM
	else if (ParamIndex == (3 + (numOfAudiotInsOuts * 2))) {
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = "FM in";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->ReadOnly = FALSE;
	}
	//entrée 0-1 pour Filter FM amount (growl)
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

}

//-----------------------------------------------------------------------------
// set the parameters events address
void filterbutterworth::onSetEventAddress(int ParamIndex, UsineEventPtr pEvent)
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
	else if (ParamIndex == (1 +numOfAudiotInsOuts  *2))
	{
		m_resonance= pEvent;
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
}

//-----------------------------------------------------------------------------
// Parameters callback
void filterbutterworth::onCallBack(UsineMessage *Message)
{
	// filter only message specific to this user module
	if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
	{
		// Message->wParam is equal to ParamIndex
		if ((Message->wParam == (numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
				cutoff = sdkGetEvtData(m_cutoff);
		}
		else if ((Message->wParam == (1 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
				resonanceRaw = sdkGetEvtData(m_resonance);		
		}
		else if ((Message->wParam == (2 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			driveRaw = sdkGetEvtData(m_drive); //drive = (sdkGetEvtData(m_drive)*5)+1;
		}
		else if ((Message->wParam == (4 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			growlRaw = sdkGetEvtData(m_growl);
			/*
			if (growlRaw > 0) {
				growlRaw = powf(1000, growlRaw)*0.0001f;
			}
			else { growlRaw = 0; }
			*/
			growl = 5000.0f * powf(growlRaw, 2);
		}
	}
}

void filterbutterworth::onProcess()
//The LP filter algo:
//out(n) = a1 * in + a2 * in(n - 1) + a3 * in(n - 2) - b1*out(n - 1) - b2*out(n - 2)
{
	sdkSmoothPrecision(cutoff, m_cutoff, 0.199f);
	//Mapping resonance range 0..1 to 0..self - osc:
	resonance = (sqrt_two - resonanceRaw * sqrt_two) * 1.8f;
	float drive = (driveRaw * 10.0f) + 1.0f;
	if (growlRaw <= 0.000001f) {
		cutoffFinal = cutoff/2;
		//cutoffFinal = cutoff * inv_samplerate;
		computeCoeff();
	}
	else {}

	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		sdkCopyEvt(audioInputs[i], audioOutputs[i]);
		//filter
		
		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			if (growlRaw > 0.000001f) {
				
				cutoffFinal = cutoff + growl * sdkGetEvtArrayData(audioFMinput, j) * cutoff / 20000.0f;
				if (cutoffFinal < 10.0f) {
				cutoffFinal = 10.0f;
				}
				else if (cutoffFinal > 20000.0f) {
				cutoffFinal = 20000.0f;
				}
				else {}
				computeCoeff();
			}
			else {}

			float input = drive * sdkGetEvtArrayData(audioInputs[i], j);
			int b_oversample = 0;
			while (b_oversample < 2) {   //2x oversampling
			signal =  a1 * input;
			signal += a2 * audioBufferN1[i];
			signal += a3 * audioBufferN2[i]; 
			signal -= b1 * audioBufferN1[i];
			signal -= b2 * audioBufferN2[i];
			signal2 = input - signal;
			//sdkTracePrecision(signal2);
			if (signal > 100.0f) { signal = 100.0f; }
			else if (signal < -100.0f) { signal = -100.0f; }
			audioBufferN2[i] = audioBufferN1[i];
			audioBufferN1[i] = signal;
			b_oversample++;
			}

			//y = x / (1 + | x | )  softclipping formula
			float out = signal2 / (1 + abs(signal2));//soft clipping
			sdkSetEvtArrayData(audioOutputs[i], j, out);


		}
		
    }
}


void filterbutterworth::computeCoeff() {

	

	//LP
	
	c = 0.5f / (tanf(PI * cutoffFinal * inv_samplerate));
	csq = c * c;


	//c = (c + resonance) * c;// = resonance*c + csq = resonance*c + c*c
	a1 = 1.0f / (1.0f + (resonance * c) + csq);
	//a1 = 1.0f / (1.0f + c);
	a2 = 2.0f * a1;
	a3 = a1;
	b1 = 2.0f * a1 *(1.0f - csq) ;
	b2 = (1.0f - (resonance * c) + csq) * a1;
	//b2 = (1.0f - c) * a1;

	//Optimization for Hipass:
/*
	c = tanf(PI * cutoff * inv_samplerate);
	csq = c * c;
	c = (c + resonance) * c;
	a1 = 1.0 / (1.0 + c);
	b1 = (1.0 - c);
*/
	//out(n) = (a1 * out(n - 1) + in - in(n - 1)) * b1;
	//HP
/*
	c = 0.5f * tan(3.1415926536f * cutoff / sdkGetSampleRate());
	csq = c * c;
	a1 = 1.0f / (1.0f + resonance * c + csq);
	a2 = -2.0f * a1;
	a3 = a1;
	b1 = 2.0f * (csq - 1.0f) * a1;
	b2 = (1.0f - resonance * c + csq) * a1;
*/	
}

/*
float filterbutterworth::map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/