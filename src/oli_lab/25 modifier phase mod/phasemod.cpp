//-----------------------------------------------------------------------------
//@file  
//	phaseModSynth.cpp
//
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
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
#include "phasemod.h"
// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "phase mod 1";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "phase modulation 1";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.1";
//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule (void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new phaseModSynth();
}

// destroy
void DestroyModule(void* pModule) 
{
	// cast is important to call the good destructor
	delete ((phaseModSynth*)pModule);
}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
phaseModSynth::phaseModSynth()
    //: coeffGain (1)
{
	// audio smooth
	//m_tevtSmoothCurrentCoeff = NULL;
}

// destructor
phaseModSynth::~phaseModSynth()
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

void phaseModSynth::onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = MODULE_NAME;
	pModuleInfo->Description = MODULE_DESC;
	pModuleInfo->Version = MODULE_VERSION;
	pModuleInfo->ModuleType         = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clAudioModuleColor) + 0x101010;
    
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
int phaseModSynth::onGetNumberOfParams (int queryIndex)
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
void phaseModSynth::onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	//
}


//-----------------------------------------------------------------------------
// initialisation
void phaseModSynth::onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) 
{
	//
}

//----------------------------------------------------------------------------
// parameters and process
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void phaseModSynth::onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo)
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
    else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts*2))
    {
		pParamInfo->ParamType		= ptAudio;
		pParamInfo->Caption			= sdkGetAudioQueryChannelNames ( "out ", ParamIndex - numOfAudiotInsOuts + 1, queryIndex);
		pParamInfo->IsInput			= FALSE;
		pParamInfo->IsOutput		= TRUE;
		pParamInfo->ReadOnly		= TRUE;

        if (ParamIndex == numOfAudiotInsOuts)
        {
            pParamInfo->IsSeparator     = TRUE;
            pParamInfo->SeparatorCaption = "audio out";
        }
    }
    // bandWidth
    else if (ParamIndex == (numOfAudiotInsOuts*2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "freq ratio";
		pParamInfo->MinValue = 0.001f;
		pParamInfo->MaxValue = 10000.0f;
		pParamInfo->DefaultValue = 1.0f;
		pParamInfo->Format = "%.5f";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
    }
    // center
    else if (ParamIndex == (numOfAudiotInsOuts*2) + 1)
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "fmod";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		pParamInfo->Format = "%.5f";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
    }
}

//-----------------------------------------------------------------------------
// set the parameters events address
void phaseModSynth::onSetEventAddress (int ParamIndex, UsineEventPtr pEvent)
{
    // audioInputs
    if (ParamIndex < numOfAudiotInsOuts)
    {
		audioInputs[ParamIndex] = pEvent;
    }
    // audioOutputs
    else if (ParamIndex >= numOfAudiotInsOuts && ParamIndex < (numOfAudiotInsOuts*2))
    {
		audioOutputs[ParamIndex - numOfAudiotInsOuts] = pEvent;
    }
    // bandwidth
    else if (ParamIndex == (numOfAudiotInsOuts*2))
	{
		m_freqRatio = pEvent;
    }
    // center
    else if (ParamIndex == (numOfAudiotInsOuts*2) + 1)
	{
		m_fmod = pEvent;
    }
}

//-----------------------------------------------------------------------------
// Parameters callback
void phaseModSynth::onCallBack (UsineMessage *Message) 
{
    // filter only message specific to this user module
    if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
    {
        // Message->wParam is equal to ParamIndex
		if ((Message->wParam == (numOfAudiotInsOuts*2)) && (Message->lParam == MSG_CHANGE))
		{
			freqRatio = sdkGetEvtData(m_freqRatio);
		}
		else if ((Message->wParam == ((numOfAudiotInsOuts * 2)+1)) && (Message->lParam == MSG_CHANGE))
		{
			fmod = sdkGetEvtData(m_fmod);
			//sdkTracePrecision(center);
		}
    }
}

void phaseModSynth::onProcess () 
{
	//float tmp = std::pow(coeffGain, 4.0f);
	//sdkSmoothEvent(m_smoothOldBandwidth, m_tevtSmoothCurrentBandwidth, bandwidth, SMOOTH); necessaire ?
	//sdkSmoothEvent(m_smoothOldCenter, m_tevtSmoothCurrentCenter, center, SMOOTH);
	for (int i = 0; i < numOfAudiotInsOuts; i++)
    {
		//sdkClearAudioEvt(audioOutputs[i]);
		sdkCopyEvt (audioInputs[i], audioOutputs[i]);
		//formula
		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			//carrier
			saw = sdkGetEvtArrayData(audioOutputs[i], j);
            //modulator
			tempValue = saw * freqRatio;
			tempValue = cos(tempValue * PI);
			tempValue = tempValue * fmod;
			saw = saw + tempValue;
			saw = cos(saw * PI);
			sdkSetEvtArrayData(audioOutputs[i], j, saw);
		}
    }
}
