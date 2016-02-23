//-----------------------------------------------------------------------------
//@file  
//	wavefolder.cpp
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//  wavefolder
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
#include "wavefolder.h"
// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "wavefolder";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "wavefolder";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.1";

//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule(void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new wavefolder();
}

// destroy
void DestroyModule(void* pModule)
{
	// cast is important to call the good destructor
	delete ((wavefolder*)pModule);

}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
wavefolder::wavefolder()
	: G(1)
{
	// audio smooth
	m_tevtSmoothCurrentCoeff = NULL;
}

// destructor
wavefolder::~wavefolder()
{
	if (m_tevtSmoothCurrentCoeff != NULL)
		sdkDestroyEvt(m_tevtSmoothCurrentCoeff);
}

// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void wavefolder::onGetModuleInfo(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = MODULE_NAME;
	pModuleInfo->Description = MODULE_DESC;
	pModuleInfo->Version = MODULE_VERSION;
	pModuleInfo->ModuleType = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clInterfaceDesignModuleColor) + 0x101010;

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
int wavefolder::onGetNumberOfParams(int queryIndex)
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
void wavefolder::onAfterQuery(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	sdkCreateEvt(m_tevtSmoothCurrentCoeff, pMasterInfo->BlocSize);
}


//-----------------------------------------------------------------------------
// initialisation
void wavefolder::onInitModule(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) {

	//----------------------------------------------------------------------------
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		/*
		"factor" depends on sampling rate and the low frequency point.
		Do not set "factor" to a fixed value(e.g. 0.99) if you don't know the sample rate.
		Instead set R to:
		(-3dB @ 40Hz) : factor = 1 - (250 / samplerate)
		(-3dB @ 30Hz) : factor = 1 - (190 / samplerate)
		(-3dB @ 20Hz) : factor = 1 - (126 / samplerate)
		*/
		fZero = 1.0f - (10.0f / sdkGetSampleRate());
		sdkClearAudioEvt(audioOutputs[i]);
		buffer[i] = 0;

		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			sdkSetEvtArrayData(audioOutputs[i], j, 0);
			audioOutputsM1[i] = sdkGetEvtArrayData(audioOutputs[i], j);
			audioInputsM1[i] = sdkGetEvtArrayData(audioInputs[i], j);

		}
	}
}
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void wavefolder::onGetParamInfo(int ParamIndex, TParamInfo* pParamInfo)
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
	// fdrGain
	else if (ParamIndex == (numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "Drive";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLog;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;

	}
	else if (ParamIndex == (1 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "positive threshold";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "negative threshold";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.3f";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (3 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "smoothing";
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
void wavefolder::onSetEventAddress(int ParamIndex, UsineEventPtr pEvent)
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
	// Gain
	else if (ParamIndex == (numOfAudiotInsOuts * 2))
	{
		m_gain = pEvent;
	}
	// pos shape
	else if (ParamIndex == (numOfAudiotInsOuts * 2) + 1)
	{
		m_pos = pEvent;
	}
	else if (ParamIndex == (numOfAudiotInsOuts * 2) + 2)
	{
		m_neg = pEvent;
	}
	else if (ParamIndex == (numOfAudiotInsOuts * 2) + 3)
	{
		m_smoothing = pEvent;
	}
}

//-----------------------------------------------------------------------------
// Parameters callback
void wavefolder::onCallBack(UsineMessage *Message)
{
	// filter only message specific to this user module
	if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
	{
		// Message->wParam is equal to ParamIndex
		if ((Message->wParam == (numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			G = sdkGetEvtData(m_gain);
		}
		else if ((Message->wParam == (1 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			a = sdkGetEvtData(m_pos);
			a = 1.0f - a;
		}
		else if ((Message->wParam == (2 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			b = sdkGetEvtData(m_neg);
			b = b - 1.0f;
		}
		else if ((Message->wParam == (3 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			smoothing = sdkGetEvtData(m_smoothing);
			//smoothing *= 2.0f;
		}
	}
}

void wavefolder::onProcess()
{
	sdkSmoothEvent(m_smoothOldCoeff, m_tevtSmoothCurrentCoeff, G, SMOOTH);

	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		sdkCopyEvt(audioInputs[i], audioOutputs[i]);

		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			G = sdkGetEvtArrayData(m_tevtSmoothCurrentCoeff, j);
			float Gain = 1.0f + 10.0f * G;
			buffer[i] = Gain * sdkGetEvtArrayData(audioInputs[i], j);
			if (buffer[i] > a) {
				buffer[i] = a + a - buffer[i];
			}
			else if (buffer[i] < b) {
				buffer[i] = b + b - buffer[i];
			}
			else {}
			buffer[i] = map(buffer[i], b, a, -1.0f, 1.0f);
			buffer[i] *= (1.0f + smoothing);
			buffer[i] = buffer[i] / (1 + fabsf(buffer[i]));
						
			//DC BLOCKER			
			sdkSetEvtArrayData(audioOutputs[i], j, (fZero * audioOutputsM1[i] - audioInputsM1[i] + buffer[i])); //remplacer 0.995 par 1.0f-(126.0f/sdkGetSampleRate())
			audioOutputsM1[i] = sdkGetEvtArrayData(audioOutputs[i], j);
			audioInputsM1[i] = buffer[i];
			
		}
		
	}
}

float wavefolder::map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
