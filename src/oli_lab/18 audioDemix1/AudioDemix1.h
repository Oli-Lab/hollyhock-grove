//-----------------------------------------------------------------------------
//@file  
//	AudioDemix1.h
//
//@author
//	Martin FLEURENT aka 'martignasse' + Oli_Lab
//
//@brief 
//	Definitions of the AudioDemix class.
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

// include once, no more
#ifndef __DEMIX_AUDIO_VOLUME_H__
#define __DEMIX_AUDIO_VOLUME_H__

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "UserDefinitions.h"  

//-----------------------------------------------------------------------------
// defines and constantes
//-----------------------------------------------------------------------------
#define AUDIO_INS_OUTS_MAX 64

//-----------------------------------------------------------------------------
// class definition
//-----------------------------------------------------------------------------
class AudioDemix : public UserModuleBase
{
    //-------------------------------------------------------------------------
	// module constructors/destructors
	//-------------------------------------------------------------------------
public:
    // constructor
	AudioDemix();

    // destructor
	virtual ~AudioDemix();

	//-------------------------------------------------------------------------
	// public methodes inherited from UserModule
	//-------------------------------------------------------------------------
public:
	//-----------------------------------------------------------------------------
	// module info
	void onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// query system and init
	int  onGetNumberOfParams( int queryIndex);
	void onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex);
	void onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// parameters and process
	void onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo);
	void onSetEventAddress (int ParamIndex, UsineEventPtr pEvent);
	void onCallBack (UsineMessage *Message);
	void onProcess ();


	//-------------------------------------------------------------------------
	// private members
	//-------------------------------------------------------------------------
private:
	//-------------------------------------------------------------------------
	// parameters events
    UsineEventPtr audioInputs[AUDIO_INS_OUTS_MAX];     // audio input
    UsineEventPtr audioOutputs1[AUDIO_INS_OUTS_MAX];    // audio output
	UsineEventPtr audioOutputs2[AUDIO_INS_OUTS_MAX];    // audio output
	UsineEventPtr fdrGain;
	UsineEventPtr switchMute;
	UsineEventPtr attenuation1;
	UsineEventPtr attenuation2;
	UsineEventPtr overlapping;

	//-------------------------------------------------------------------------
    static const int numOfParamAfterAudiotInOut = 5;

	int queryIndex;
	int numOfAudiotInsOuts;

    TPrecision Gain;
	TPrecision Gain1;
	TPrecision Gain2;
	TPrecision mute;

}; // class AudioDemix

#endif //__EXAMPLE_AUDIO_VOLUME_H__