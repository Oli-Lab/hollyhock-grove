//-----------------------------------------------------------------------------
//@file  
//	DoublePendulum.h
//
//@author
//	Olivar Premier
//
//@brief 
//	Definitions of the helloWorldModule class.
//
//  A template include to start from for new user module.
//
//@historic 
//	2013/05/15
//    first release for Hollyhock CPP SDK 6.00.226 
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
// Copyright (C) 2013 Sensomusic
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
#ifndef __DOUBLE_PENDULUM_MODULE_H__
#define __DOUBLE_PENDULUM_MODULE_H__
//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "UserDefinitions.h"  
//#include <iostream>
//#include <cmath>
//#include <limits>

//-----------------------------------------------------------------------------
// defines and constantes
//-----------------------------------------------------------------------------

// defines and constantes goes here 


//-----------------------------------------------------------------------------
// structures and typedef
//-----------------------------------------------------------------------------

// structures and typedef goes here

//-----------------------------------------------------------------------------
// class definition
//-----------------------------------------------------------------------------
class DoublePendulum : public UserModuleBase
{
    //-------------------------------------------------------------------------
	// module constructors/destructors
	//-------------------------------------------------------------------------
public:
    // constructor
	DoublePendulum();

    // destructor
	virtual ~DoublePendulum();

	//-------------------------------------------------------------------------
	// public methodes inherited from UserModule
	//-------------------------------------------------------------------------
public:
	//-----------------------------------------------------------------------------
	// module info
	void onGetModuleInfo (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// query system and init
	int  onGetNumberOfParams( int QIdx);
	void onAfterQuery (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int QIdx);
	void onInitModule (MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo);

	//-----------------------------------------------------------------------------
	// parameters and process
	void onGetParamInfo (int ParamIndex, TParamInfo* pParamInfo);
	void onSetEventAddress (int ParamIndex, UsineEventPtr pEvent);
	void onCallBack (UsineMessage *Message);
	void onProcess ();

	//-----------------------------------------------------------------------------
	// midi out callbacks
	void onMidiSendOut (int DeviceID, UsineMidiCode Code);
	void onMidiSysexSendOut (int DeviceID, char* Sysex);

	//-----------------------------------------------------------------------------
	// chunk system
	int  onGetChunkLen (LongBool Preset);
	void onGetChunk (void* chunk, LongBool Preset);
	void onSetChunk (const void* chunk, int sizeInBytes, LongBool Preset);

	//-----------------------------------------------------------------------------
	// layout
	void onCreateSettings();
	void onSettingsHasChanged();
    //void onResize (float contentWidth, float contentHeight);
    //void onCreateCommands();
	void onPaint ();

	//-----------------------------------------------------------------------------
	// mouse and multi touch interaction
	void onMouseMove(TShiftState Shift, float X, float Y);
	void onMouseDown(TMouseButton MouseButton, TShiftState Shift, float X,float Y);
	void onMouseUp (TMouseButton MouseButton, TShiftState Shift, float X,float Y);
	void onMouseMoveMultiProc(TShiftState Shift, UsineEventPtr X, UsineEventPtr Y);
	void onMouseDownMultiProc(TMouseButton MouseButton, TShiftState Shift, UsineEventPtr X, UsineEventPtr Y);
	void onMouseUpMultiProc (TMouseButton MouseButton, TShiftState Shift,UsineEventPtr X, UsineEventPtr Y);
	void onOpenEditor();
	void onBringToFront();

	//-----------------------------------------------------------------------------
	// audio setup update
	void onBlocSizeChange (int BlocSize);
	void onSampleRateChange (double SampleRate);
	
	//-----------------------------------------------------------------------------
	// recording 
	void onSetRecordedValue (TPrecision X, TPrecision Y, TPrecision Z);

	//-------------------------------------------------------------------------
	// public methodes
	//-------------------------------------------------------------------------
public:

    // public methodes goes here

	//-------------------------------------------------------------------------
	// protected members
	//-------------------------------------------------------------------------
protected:
	//-------------------------------------------------------------------------
	// parameters events : 
	UsineEventPtr m_L1;       // pendulum 1 length
	UsineEventPtr m_L2;       // pendulum 2 length
	UsineEventPtr m_M1;       // pendulum 1 Mass
	UsineEventPtr m_M2;       // pendulum 2 Mass
	UsineEventPtr m_friction;      
	UsineEventPtr m_gravity;  
	UsineEventPtr m_gravityAngle;
	UsineEventPtr m_physicSpeed;
	UsineEventPtr m_reset; 
	UsineEventPtr m_clockIn;
	UsineEventPtr m_M1posx;  
	UsineEventPtr m_M1posy;
	UsineEventPtr m_M2posx;
	UsineEventPtr m_M2posy;
	UsineEventPtr m_OffSetM1posx;
	UsineEventPtr m_OffSetM1posy;
	UsineEventPtr m_OffSetM2posx;
	UsineEventPtr m_OffSetM2posy;
	//UsineEventPtr m_angleM1;
	//UsineEventPtr m_angleM2;

	// protected members goes here
	const float centerx = 0.5;
	const float centery = 0.5;             //anchor point of the pendulum
	float newAngle = 0;
	float x[4];// = { 0, 0, 0, 0 };      //state vector,  state vars are theta1 theta1' theta2 theta2'
	float dx[4];// = { 0, 0, 0, 0 };    //for storing the value of the state function at the current state
	float gravityAngle = 0;
	float gravityCorrection = 0;
	float prev_gravityCorrection = 0;
	int token1 = 1;
  
	//-------------------------------------------------------------------------
	// private methodes
	//-------------------------------------------------------------------------
private:  

    // private methodes goes here
	

}; // class TemplateModule

#endif //__DOUBLE_PENDULUM_MODULE_H__