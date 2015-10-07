//-----------------------------------------------------------------------------
//@file  
//	AudioVolumeExample.cpp
//
//@author
//	Martin FLEURENT aka 'martignasse' modified by Oli_Lab
//
//@brief 
//	Implementation of the AudioVolumeExample class.
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
#include "AudioVolumeDCblock.h"

//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule (void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
{
	pModule = new AudioVolumeDCblock();
}

// destroy
void DestroyModule(void* pModule) 
{
	// cast is important to call the good destructor
	delete ((AudioVolumeDCblock*)pModule);

}

//-------------------------------------------------------------------------
// module constructors/destructors
//-------------------------------------------------------------------------

// constructor
AudioVolumeDCblock::AudioVolumeDCblock()
    : coeffGain (1)
{
	// audio smooth
	m_tevtSmoothCurrentCoeff = NULL;
}

// destructor
AudioVolumeDCblock::~AudioVolumeDCblock()
{
	if (m_tevtSmoothCurrentCoeff != NULL)
		sdkDestroyEvt(m_tevtSmoothCurrentCoeff);
}

void AudioVolumeDCblock::onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name				= "DC block";
	pModuleInfo->Description		= "audio volume and DC blocker";
	pModuleInfo->ModuleType         = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clInterfaceDesignModuleColor) + 0x101010;
	pModuleInfo->Version			= "2.1";
    
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
int AudioVolumeDCblock::onGetNumberOfParams (int queryIndex)
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
void AudioVolumeDCblock::onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	sdkCreateEvt(m_tevtSmoothCurrentCoeff, pMasterInfo->BlocSize);
}


//-----------------------------------------------------------------------------
// initialisation
void AudioVolumeDCblock::onInitModule(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) {

	//----------------------------------------------------------------------------
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{
		//sdkCopyEvt(audioInputs[i], audioOutputs[i]);
		sdkClearAudioEvt(audioOutputs[i]);

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
void AudioVolumeDCblock::onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo)
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
    // fdrGain
    else if (ParamIndex == (numOfAudiotInsOuts*2))
	{
		pParamInfo->ParamType		= ptGainFader;
		pParamInfo->Caption			= "gain";
		pParamInfo->IsInput			= TRUE;
		pParamInfo->IsOutput		= FALSE;
        pParamInfo->IsSeparator     = TRUE;
        pParamInfo->CallBackType    = ctImmediate;
        pParamInfo->SeparatorCaption = "Volume";

    }
    // switchMute
    else if (ParamIndex == (numOfAudiotInsOuts*2) + 1)
	{
		pParamInfo->ParamType		= ptLeftLed;
		pParamInfo->Caption			= "mute";
		pParamInfo->IsInput			= TRUE;
		pParamInfo->IsOutput		= FALSE;
        pParamInfo->CallBackType    = ctImmediate;
    }
}

//-----------------------------------------------------------------------------
// set the parameters events address
void AudioVolumeDCblock::onSetEventAddress (int ParamIndex, UsineEventPtr pEvent)
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
    // fdrGain
    else if (ParamIndex == (numOfAudiotInsOuts*2))
	{
		fdrGain = pEvent;
    }
    // switchMute
    else if (ParamIndex == (numOfAudiotInsOuts*2) + 1)
	{
		switchMute = pEvent;
    }
}

//-----------------------------------------------------------------------------
// Parameters callback
void AudioVolumeDCblock::onCallBack (UsineMessage *Message) 
{
    // filter only message specific to this user module
    if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
    {
        // Message->wParam is equal to ParamIndex
		if ((Message->wParam >= (numOfAudiotInsOuts*2)) && (Message->lParam == MSG_CHANGE))
		{
			coeffGain = sdkDbToCoeff (sdkGetEvtData (fdrGain)) * (1.0f - sdkGetEvtData (switchMute));
		}
    }
}

void AudioVolumeDCblock::onProcess () 
{
	sdkSmoothEvent(m_smoothOldCoeff, m_tevtSmoothCurrentCoeff, coeffGain, SMOOTH);
	for (int i = 0; i < numOfAudiotInsOuts; i++)
    {
		sdkCopyEvt(audioInputs[i], audioOutputs[i]);
		//sdkClearAudioEvt(audioOutputs[i]);
				
		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			//audioOutputsM1[i] = 0.995f*audioOutputsM1[i] - audioInputsM1[i] + sdkGetEvtArrayData(audioInputs[i], j);
			sdkSetEvtArrayData(audioOutputs[i], j, (0.995f*audioOutputsM1[i] - audioInputsM1[i] + sdkGetEvtArrayData(audioInputs[i],j)));
			//sdkSetEvtArrayData(audioOutputs[i], j, audioOutputsM1[i]);
			audioOutputsM1[i] = sdkGetEvtArrayData(audioOutputs[i], j);
			audioInputsM1[i] = sdkGetEvtArrayData(audioInputs[i], j);
			//y = x - xm1 + 0.995 * ym1;
			//xm1 = x;
			//ym1 = y;
		}

			sdkMultEvt2Audio(m_tevtSmoothCurrentCoeff, audioOutputs[i]);
    }
}
