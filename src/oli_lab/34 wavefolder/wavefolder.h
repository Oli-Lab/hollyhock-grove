//-----------------------------------------------------------------------------
//@file  
//	wavefolder.h
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//  wavefolder
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
#ifndef __WAVE_FOLDER_H__
#define __WAVE_FOLDER_H__

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "../../sdk/UserDefinitions.h"  

//-----------------------------------------------------------------------------
// defines and constantes
//-----------------------------------------------------------------------------
#define AUDIO_INS_OUTS_MAX 64
const float SMOOTH = 0.99f;
//const float SMOOTH_FAST = 0.11f;
//-----------------------------------------------------------------------------
// class definition
//-----------------------------------------------------------------------------
class wavefolder : public UserModuleBase
{
	//-------------------------------------------------------------------------
	// module constructors/destructors
	//-------------------------------------------------------------------------
public:
	// constructor
	wavefolder();

	// destructor
	virtual ~wavefolder();

	//-------------------------------------------------------------------------
	// public methodes inherited from UserModule
	//-------------------------------------------------------------------------
public:
	//-----------------------------------------------------------------------------
	// module info
	void onGetModuleInfo(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// query system and init
	int  onGetNumberOfParams(int queryIndex);
	void onAfterQuery(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex);
	void onInitModule(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// parameters and process
	void onGetParamInfo(int ParamIndex, TParamInfo* pParamInfo);
	void onSetEventAddress(int ParamIndex, UsineEventPtr pEvent);
	void onCallBack(UsineMessage *Message);
	void onProcess();


	//-------------------------------------------------------------------------
	// private members
	//-------------------------------------------------------------------------
private:
	float map(float x, float in_min, float in_max, float out_min, float out_max);
	//-------------------------------------------------------------------------
	// parameters events
	
	UsineEventPtr audioInputs[AUDIO_INS_OUTS_MAX];     // audio input
	UsineEventPtr audioOutputs[AUDIO_INS_OUTS_MAX];    // audio output
	TPrecision audioInputsM1[AUDIO_INS_OUTS_MAX];      // audio input at t-1
	TPrecision audioOutputsM1[AUDIO_INS_OUTS_MAX];     // audio output at t-1
	TPrecision buffer[AUDIO_INS_OUTS_MAX];
	UsineEventPtr m_gain;
	UsineEventPtr m_pos;
	UsineEventPtr m_neg;
	UsineEventPtr m_smoothing;
	TPrecision G;
	TPrecision a;
	TPrecision b;
	TPrecision smoothing;
	// for audio smooth
	TPrecision m_smoothOldCoeff = 0;
	UsineEventPtr m_tevtSmoothCurrentCoeff;
	
	TPrecision fZero;

	//-------------------------------------------------------------------------
	static const int numOfParamAfterAudiotInOut = 4;

	int queryIndex;
	int numOfAudiotInsOuts;

}; // class AudioVolumeExample

#endif //__EXAMPLE_AUDIO_VOLUME_H__