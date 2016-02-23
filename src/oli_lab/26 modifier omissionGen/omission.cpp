//-----------------------------------------------------------------------------
//@file  
//	omission.cpp
//
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
// every n square events, the generator omit to transmit the square...
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
#include "omission.h"
// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "omission gen 1";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "omission generator";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.1";
//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule (void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new synth();
}

// destroy
void DestroyModule(void* pModule) 
{
	// cast is important to call the good destructor
	delete ((synth*)pModule);
}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
synth::synth()
    //: coeffGain (1)
{
	// audio smooth
	//m_tevtSmoothCurrentCoeff = NULL;
}

// destructor
synth::~synth()
{
	//if (m_tevtSmoothCurrentCoeff != NULL)
	//	sdkDestroyEvt(m_tevtSmoothCurrentCoeff);
}

// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void synth::onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
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
	    pModuleInfo->QueryDefaultIdx	= 0;
    }
}

//-----------------------------------------------------------------------------
// query system and init methodes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Get total parameters number of the module
int synth::onGetNumberOfParams (int queryIndex)
{
	int result = 0;
    this->queryIndex = queryIndex;
    numOfAudiotInsOuts = sdkGetAudioQueryToNbChannels (queryIndex);

    // we want 1 in 1 out per channels
	result = (numOfAudiotInsOuts * 5) + numOfParamAfterAudiotInOut;

    return result;
}

//-----------------------------------------------------------------------------
// Called after the query popup
void synth::onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	//
}


//-----------------------------------------------------------------------------
// initialisation
void synth::onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) 
{
	//
}

//----------------------------------------------------------------------------
// parameters and process
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void synth::onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo)
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
            pParamInfo->SeparatorCaption = "saw in";
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
		else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 3)){
			pParamInfo->ParamType = ptDataFader;
			pParamInfo->Caption = sdkGetAudioQueryChannelNames("detect", ParamIndex - (numOfAudiotInsOuts*2) + 1, queryIndex);
			pParamInfo->MinValue = 0;
			pParamInfo->MaxValue = 1;
			pParamInfo->DefaultValue = 0;
			pParamInfo->Format = "%.0f";
			pParamInfo->IsInput = FALSE;
			pParamInfo->IsOutput = TRUE;
			pParamInfo->CallBackType = ctImmediate;
    }
    // divider factor
    else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 4)) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "div factor";
		pParamInfo->MinValue = 2;
		pParamInfo->MaxValue = 100;
		pParamInfo->DefaultValue = 2;
		pParamInfo->Format = "%.0f";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
    }
		// PWM factor
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 5)) {
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "PWM";
pParamInfo->MinValue = 0;
pParamInfo->MaxValue = 1;
pParamInfo->DefaultValue = 0;
pParamInfo->Format = "%0.5f";
pParamInfo->IsInput = TRUE;
pParamInfo->IsOutput = FALSE;
pParamInfo->CallBackType = ctImmediate;
	}
}

//-----------------------------------------------------------------------------
// set the parameters events address
void synth::onSetEventAddress(int ParamIndex, UsineEventPtr pEvent)
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
	// detect out
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 3))
	{
		m_divider[ParamIndex - numOfAudiotInsOuts * 2] = pEvent;
	}
	// div factor
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 4))
	{
		m_divFactor[ParamIndex - numOfAudiotInsOuts * 3] = pEvent;
	}
	// PWM
	else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts * 5))
	{
		m_PWM[ParamIndex - numOfAudiotInsOuts * 4] = pEvent;
	}

}

//-----------------------------------------------------------------------------
// Parameters callback
void synth::onCallBack(UsineMessage *Message)
{
	// filter only message specific to this user module
	if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
	{
		// Message->wParam is equal to ParamIndex
		if ((Message->wParam < (numOfAudiotInsOuts * 4)) && (Message->lParam == MSG_CHANGE))
		{
			for (int i = 0; i < numOfAudiotInsOuts; i++) {
				divFactor[i] = sdkGetEvtData(m_divFactor[i]);
			}

		}
		else if ((Message->wParam < (numOfAudiotInsOuts * 5)) && (Message->lParam == MSG_CHANGE))
		{
			for (int i = 0; i < numOfAudiotInsOuts; i++) {
				PWM[i] = sdkGetEvtData(m_PWM[i]);
				if (PWM[i] > 0.9999) {
					PWM[i] = 0.9999;
				}
			}
		}
	}
}

void synth::onProcess()
{
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		sdkCopyEvt(audioInputs[i], audioOutputs[i]);
		//formula
		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			//make a square signal
			signal = sdkGetEvtArrayData(audioOutputs[i], j);
			if (signal >= PWM[i]) {
				signal = 1;
				if (PreviousState[i] != signal) {
					PreviousState[i] = 1;
					counter[i]++;
				}
			}
			else if (signal < PWM[i]) {
				signal = -1;
				if (PreviousState[i] != signal) {
					PreviousState[i] = 0;
				}
			}
			else {}
			//counter
			if (counter[i] >= (divFactor[i] + 1)) {
				counter[i] = 0;
				//sdkSetEvtData(m_divider[i], 0);
			}
			else if (counter[i] >= divFactor[i]) {
				signal = -1;
				if (detect[i] == 0){
				detect[i] = 1;
			    }
				else if (detect[i] >= 1) {
				detect[i] = 0;
				}	
			}
			else {}
            //output
			sdkSetEvtArrayData(audioOutputs[i], j, signal);
		}
		sdkSetEvtData(m_divider[i], detect[i]);
    }
}
