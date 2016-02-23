//-----------------------------------------------------------------------------
//@file  
//	filter.cpp
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//  Multimode State Variable Filterwith 2 output (2 poles)
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
const AnsiCharPtr UserModuleBase::MODULE_NAME = "state variable MultiMode 2o";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "state variable MultiMode 2o";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.1";
//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule (void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
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
	: driveRaw(1)
{
	// audio smooth
	m_tevtSmoothCurrentDrive = NULL;
}

// destructor
filter::~filter()
{
	if (m_tevtSmoothCurrentDrive != NULL)
		sdkDestroyEvt(m_tevtSmoothCurrentDrive);
}

// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void filter::onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
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
int filter::onGetNumberOfParams (int queryIndex)
{
	int result = 0;
    this->queryIndex = queryIndex;
    numOfAudiotInsOuts = sdkGetAudioQueryToNbChannels (queryIndex);

    // we want 1 in 1 out per channels
	result = (numOfAudiotInsOuts * 3) + numOfParamAfterAudiotInOut;

    return result;
}

//-----------------------------------------------------------------------------
// Called after the query popup
void filter::onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	sdkCreateEvt(m_tevtSmoothCurrentDrive, pMasterInfo->BlocSize);
}


//-----------------------------------------------------------------------------
// initialisation
void filter::onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) 
{
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
			audioBufferD2[i] = 0;
			audioBufferD1[i] = 0;
			inv_samplerate = 1/sdkGetSampleRate();
			resonanceRaw = 0;
			driveRaw = 0;
	}
	FILTER[0] = 0;
	FILTER[1] = 0;
	FILTER[2] = 0;
	FILTER[3] = 0;
}

//----------------------------------------------------------------------------
// parameters and process
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void filter::onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo)
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
    // audioOutputs A
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
			pParamInfo->SeparatorCaption = "audio out A";
		}
	}

	// audioOutputs B
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 3))
	{
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = sdkGetAudioQueryChannelNames("out ", ParamIndex - (numOfAudiotInsOuts + 1)*2, queryIndex);
		pParamInfo->IsInput = FALSE;
		pParamInfo->IsOutput = TRUE;
		pParamInfo->ReadOnly = TRUE;

		if (ParamIndex == (numOfAudiotInsOuts*2))
		{
			pParamInfo->IsSeparator = TRUE;
			pParamInfo->SeparatorCaption = "audio out B";
		}
	}

    // filter cutoff
    else if (ParamIndex == numOfAudiotInsOuts*3) {
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
	else if (ParamIndex == (1 + (numOfAudiotInsOuts * 3))) {
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

	else if (ParamIndex == (2 + (numOfAudiotInsOuts * 3))) {
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
	else if (ParamIndex == (3 + (numOfAudiotInsOuts * 3))) {
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = "FM in";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->ReadOnly = FALSE;
	}
	//prevoir entrée 0-1 pour Filter FM amount (growl)
	else if (ParamIndex == (4 + (numOfAudiotInsOuts * 3))) {
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

	else if (ParamIndex == (5 + (numOfAudiotInsOuts * 3))) {
		pParamInfo->ParamType = ptListBox;
		pParamInfo->Caption = "F type A";
		pParamInfo->ListBoxStrings = "\"Lo Pass>>A\",\"Hi Pass>>A\",\"Band Pass>>A\",\"Notch>>A\"";
		pParamInfo->DefaultValue = 0;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		//pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (6 + (numOfAudiotInsOuts * 3))) {
		pParamInfo->ParamType = ptListBox;
		pParamInfo->Caption = "F type B";
		pParamInfo->ListBoxStrings = "\"Lo Pass>>B\",\"Hi Pass>>B\",\"Band Pass>>B\",\"Notch>>B\"";
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
	// audioOutputs A
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 2))
	{
		audioOutputs1[ParamIndex - numOfAudiotInsOuts] = pEvent;
	}
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 3))
	{
		audioOutputs2[ParamIndex - numOfAudiotInsOuts*2] = pEvent;
	}
	// cutoff
	else if (ParamIndex == numOfAudiotInsOuts * 3)
	{
		m_cutoff = pEvent;
	}
	// resonance
	else if (ParamIndex == (1 +numOfAudiotInsOuts  *3))
	{
		m_resonance= pEvent;
	}
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 3))
	{
		m_drive = pEvent;
	}
	else if (ParamIndex == (3 + numOfAudiotInsOuts * 3))
	{
		audioFMinput = pEvent;
	}
	else if (ParamIndex == (4 + numOfAudiotInsOuts * 3))
	{
		m_growl = pEvent;
	}
	else if (ParamIndex == (5 + numOfAudiotInsOuts * 3))
	{
		m_typeA = pEvent;
	}
	else if (ParamIndex == (6 + numOfAudiotInsOuts * 3))
	{
		m_typeB = pEvent;
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
		if ((Message->wParam == (numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
				cutoff = sdkGetEvtData(m_cutoff);
		}
		else if ((Message->wParam == (1 + numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
				resonanceRaw = sdkGetEvtData(m_resonance);		
		}
		else if ((Message->wParam == (2 + numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
			driveRaw = sdkGetEvtData(m_drive);
		}
		else if ((Message->wParam == (4 + numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
			growlRaw = sdkGetEvtData(m_growl); 
			//growl = 5000.0f * powf(growlRaw, 2);
			if (growlRaw > 0) {
				growlRaw = powf(1000, growlRaw)*0.0001f;
			}
			else { growlRaw = 0; }

		}
		else if ((Message->wParam == (5 + numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
			typeA = sdkGetEvtData(m_typeA);
		}
		else if ((Message->wParam == (6 + numOfAudiotInsOuts * 3)) && (Message->lParam == MSG_CHANGE))
		{
			typeB = sdkGetEvtData(m_typeB);
		}
		
	}
}

void filter::onProcess() {
	sdkSmoothPrecision(cutoff, m_cutoff, 0.1f);
	//sdkSmoothPrecision(driveRaw, m_drive, 0.999f);
	
	resonance = (1.0f - resonanceRaw) * 2.0f; // from 2 to 0
	float drive = (driveRaw * 2.0f) + 1.0f;
	sdkSmoothEvent(m_smoothOldDrive, m_tevtSmoothCurrentDrive, drive, 0.999f);

	if (growlRaw <= 0.000001f) {
		cutoffFinal = cutoff * inv_samplerate;
		//cutoffFinal = cutoff;
		computeCoeff();
	}
	else {}

	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		sdkCopyEvt(audioInputs[i], audioOutputs1[i]);
		sdkCopyEvt(audioInputs[i], audioOutputs2[i]);
		sdkMultEvt2Audio(m_tevtSmoothCurrentDrive, audioInputs[i]);
		//filter
		
		for (int j = 0; j < sdkGetEvtSize(audioOutputs1[i]); j++)
		{
			if (growlRaw > 0.000001f) {
				/*
				cutoffFinal = cutoff + growl * sdkGetEvtArrayData(audioFMinput, j) * cutoff / 10000.0f;
				if (cutoffFinal < 10.0f) { 
					cutoffFinal = 10.0f; 
				}
				else if (cutoffFinal > 20000.0f) {
					cutoffFinal = 20000.0f;
				}
				else {}
				*/
				growl = growlRaw * sdkGetEvtArrayData(audioFMinput, j);
				cutoffFinal = cutoff * inv_samplerate;
				cutoffFinal += growl;
				cutoffFinal = cutoffFinal / (1 + abs(cutoffFinal));//soft clipping
				if (cutoffFinal < 0) { cutoffFinal = 0; }
				computeCoeff();
			}
			else {}
			/*
			// algorithm
			// loop
			L = D2 + F1 * D1  //
			H = I - L - Q1*D1
			B = F1 * H + D1
			N = H + L
    
			// store delays
			D1 = B
			D2 = L
			
			*/
			//int b_oversample = 0;
			//float b_inSH = sdkGetEvtArrayData(audioInputs[i], j); // before the while statement.
			//while (b_oversample < 4) {                        //2x oversampling (@44.1khz)		
				float c_x_D1 = c * audioBufferD1[i];
				c_x_D1 = c_x_D1 / (1 + abs(c_x_D1));//soft clipping
				FILTER[0] = audioBufferD2[i] + c_x_D1; //L
				FILTER[1] = sdkGetEvtArrayData(audioInputs[i], j) - FILTER[0] - resonance * audioBufferD1[i]; //le produit res * D1 pourrait passer par un soft clipper!?
				//H = drive * b_inSH - L - resonance * audioBufferD1[i]; //le produit res * D1 pourrait passer par un soft clipper!?
				float c_x_H = c * FILTER[1];
				c_x_H = c_x_H / (1 + abs(c_x_H));//soft clipping
				FILTER[2] = c_x_H + audioBufferD1[i];
				if (FILTER[0] > 1.0f) { FILTER[0] = 1.0f; } //cette valeur pourrait être augmentée !?
				else if (FILTER[0] < -1.0f) { FILTER[0] = -1.0f; }
				else {}
				FILTER[3] = FILTER[1] + FILTER[0];
				audioBufferD1[i] = FILTER[2];
				audioBufferD2[i] = FILTER[0];
				//b_oversample++;
			//}
			float out = FILTER[typeA] / (1 + abs(FILTER[typeA]));//soft clipping
			sdkSetEvtArrayData(audioOutputs1[i], j, out);
			out = FILTER[typeB] / (1 + abs(FILTER[typeB]));//soft clipping
			sdkSetEvtArrayData(audioOutputs2[i], j, 0.7f * out);
		}
    }
}


void filter::computeCoeff() {	
	c = sinf(PI * cutoffFinal ) * 2; //* inv_samplerate
}

/*
float filter::map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/