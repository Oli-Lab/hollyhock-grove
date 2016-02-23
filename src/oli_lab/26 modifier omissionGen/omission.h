//-----------------------------------------------------------------------------
//@file  
//	omission.h
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

// include once, no more
#ifndef __OMISSION_SYNTH_H__
#define __OMISSION_SYNTH_H__

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "../../sdk/UserDefinitions.h"  

//-----------------------------------------------------------------------------
// defines and constantes
//-----------------------------------------------------------------------------
#define AUDIO_INS_OUTS_MAX 64
//const float SMOOTH = 0.99f;
//-----------------------------------------------------------------------------
// class definition
//-----------------------------------------------------------------------------
class synth : public UserModuleBase
{
    //-------------------------------------------------------------------------
	// module constructors/destructors
	//-------------------------------------------------------------------------
public:
    // constructor
	synth();

    // destructor
	virtual ~synth();

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
    UsineEventPtr audioOutputs[AUDIO_INS_OUTS_MAX];    // audio output
	int PreviousState[AUDIO_INS_OUTS_MAX];    // state of each square signal for counting purpose
	int counter[AUDIO_INS_OUTS_MAX];
	int detect[AUDIO_INS_OUTS_MAX];
	TPrecision tempValue;
	TPrecision signal;
	///////////////////////////:
	UsineEventPtr m_divFactor[AUDIO_INS_OUTS_MAX];
	UsineEventPtr m_PWM[AUDIO_INS_OUTS_MAX];
	UsineEventPtr m_divider[AUDIO_INS_OUTS_MAX];
	TPrecision divFactor[AUDIO_INS_OUTS_MAX];
	TPrecision PWM[AUDIO_INS_OUTS_MAX];
	TPrecision divider[AUDIO_INS_OUTS_MAX];
	

	//-------------------------------------------------------------------------
    static const int numOfParamAfterAudiotInOut = 0;

	int queryIndex;
	int numOfAudiotInsOuts;
    

}; // class synth

#endif //__EXAMPLE_AUDIO_VOLUME_H__