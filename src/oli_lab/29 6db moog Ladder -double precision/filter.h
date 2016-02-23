//-----------------------------------------------------------------------------
//@file  
//	filter.h
//
//@author
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//@brief 
//	Definitions of the filter class.
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
#ifndef __FILTER_H__
#define  __FILTER_H__

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
class filter : public UserModuleBase
{
	//-------------------------------------------------------------------------
	// module constructors/destructors
	//-------------------------------------------------------------------------
public:
	// constructor
	filter();

	// destructor
	virtual ~filter();

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
	void computeCoeff();
	//float map(float x, float in_min, float in_max, float out_min, float out_max);


	//-------------------------------------------------------------------------
	// private members
	//-------------------------------------------------------------------------
private:
	//-------------------------------------------------------------------------
	// parameters events
	//float c, csq, resonance, q, a0, a1, a2, b1, b2;
	UsineEventPtr audioInputs [AUDIO_INS_OUTS_MAX];    // audio input
	UsineEventPtr audioOutputs[AUDIO_INS_OUTS_MAX];    // audio output
	UsineEventPtr audioFMinput;    // filter FM audio input
	TPrecision audioBuffer0 [AUDIO_INS_OUTS_MAX];
	TPrecision audioBuffer1 [AUDIO_INS_OUTS_MAX];
	TPrecision audioBuffer2 [AUDIO_INS_OUTS_MAX];
	TPrecision audioBuffer3 [AUDIO_INS_OUTS_MAX];
	TPrecision audioBuffer4 [AUDIO_INS_OUTS_MAX];
	TPrecision resonanceBuffer[AUDIO_INS_OUTS_MAX];

	TPrecision cutoff;
	double cutoffFinal;
	double resonanceRaw;
	double resonance;
	double drive;
	TPrecision driveRaw;
	TPrecision growlRaw;
	double growl;
	double f;
	double k; //cutoff corrected resonance coeff
	double p; //cutoff corrected resonance coeff
	double scale; //cutoff corrected resonance coeff
	//dc blocker
	double factor;
	double y[AUDIO_INS_OUTS_MAX];
	double xm1[AUDIO_INS_OUTS_MAX];
	double ym1[AUDIO_INS_OUTS_MAX];
	//
	double t1;
	double t2;
	double in;

	double inv_samplerate;

	///////////////////////////:
	UsineEventPtr m_cutoff;
	UsineEventPtr m_resonance;
	UsineEventPtr m_drive;
	UsineEventPtr m_growl;
	UsineEventPtr m_type;
	UsineEventPtr m_feedback;
	int type;
	int feedback;

	int mangled;
	int count;
	double G = 1.5f;
	double a = 1.5f;
	double b = 0.1f;

	TPrecision m_smoothOldDrive = 0;
	UsineEventPtr m_tevtSmoothCurrentDrive;
	const double sqrtdivPI = 0.4501581581f;


	//-------------------------------------------------------------------------
	static const int numOfParamAfterAudiotInOut = 7;

	int queryIndex;
	int numOfAudiotInsOuts;


}; // class filter

#endif //