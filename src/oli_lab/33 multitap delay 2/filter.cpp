//-----------------------------------------------------------------------------
//@file  
//	multitap delay.cpp
//
//  Oli_Lab with precious help from
//	Martin FLEURENT aka 'martignasse'
//
//@brief 
//	Implementation of the multitap delay class.
//
//  a delay line with up to 64 taps !
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
//#include "denormals.h"

// module constants for browser info and module info
const AnsiCharPtr UserModuleBase::MODULE_NAME = "HydroGen OP";
const AnsiCharPtr UserModuleBase::MODULE_DESC = "HydroGen OP";
const AnsiCharPtr UserModuleBase::MODULE_VERSION = "0.1";

//----------------------------------------------------------------------------
// create, general info and destroy methodes
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Create
void CreateModule(void* &pModule, AnsiCharPtr optionalString, LongBool Flag, MasterInfo* pMasterInfo, AnsiCharPtr optionalContent)
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
{
	//Initialize values
	m_size = lineSize * 48000; //longueur fixe pour l'instant
	m_read[AUDIO_INS_OUTS_MAX] = 0;
	m_write = m_size - 1;

	//Ensure that initial sample values at buffer are 0.0f
	m_buffer = new float[m_size];
	for (int i = 0; i<m_size; i++)
	{
		m_buffer[i] = 0.0f;
	}
}

// destructor
filter::~filter()
{
	delete[] m_buffer;
}


// browser info
void GetBrowserInfo(ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = UserModuleBase::MODULE_NAME;
	pModuleInfo->Description = UserModuleBase::MODULE_DESC;
	pModuleInfo->Version = UserModuleBase::MODULE_VERSION;
}

void filter::onGetModuleInfo(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo)
{
	pModuleInfo->Name = MODULE_NAME;
	pModuleInfo->Description = MODULE_DESC;
	pModuleInfo->Version = MODULE_VERSION;
	pModuleInfo->ModuleType = mtSimple;
	pModuleInfo->BackColor = sdkGetUsineColor(clInterfaceDesignModuleColor) + 0x101010;
	pModuleInfo->NumberOfParams = 3;
	
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
int filter::onGetNumberOfParams(int queryIndex)
{
	
	int result = 0;
	this->queryIndex = queryIndex;
	numOfAudiotInsOuts = sdkGetAudioQueryToNbChannels(queryIndex);
	// we want 1 in and 1 out per channels
	result = 1 + (numOfAudiotInsOuts * 2) + numOfParamAfterAudiotInOut;
	return result;
	
}

//-----------------------------------------------------------------------------
// Called after the query popup
void filter::onAfterQuery(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo, int queryIndex)
{
	//
}


//-----------------------------------------------------------------------------
// initialisation
void filter::onInitModule(MasterInfo* pMasterInfo, ModuleInfo* pModuleInfo) {

	//----------------------------------------------------------------------------
	for (int i = 0; i < numOfAudiotInsOuts; i++)
	{

		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{
			sdkSetEvtArrayData(audioOutputs[i], j, 0);
		}
	m_delaySmoothed[i] = 0;
	tapOutOld[i] = 0;
	}
	SampleRate = sdkGetSampleRate();
	max_size = lineSize * SampleRate;
	blocSize = sdkGetBlocSize();


}
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Parameters description
void filter::onGetParamInfo(int ParamIndex, TParamInfo* pParamInfo)
{
	// audioInputs
	if (ParamIndex == 0)
	{
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = "in";
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->ReadOnly = FALSE;
		/*
		if (ParamIndex == 0)
		{
		pParamInfo->IsSeparator = TRUE;
		pParamInfo->SeparatorCaption = "audio in";
		}
		*/
	}
	// audioOutputs
	else if (ParamIndex < (numOfAudiotInsOuts + 1))
	{
		pParamInfo->ParamType = ptAudio;
		pParamInfo->Caption = sdkGetAudioQueryChannelNames("out ", ParamIndex - 1, queryIndex);
		pParamInfo->IsInput = FALSE;
		pParamInfo->IsOutput = TRUE;
		pParamInfo->ReadOnly = TRUE;

		if (ParamIndex == 1)
		{
			pParamInfo->IsSeparator = TRUE;
			pParamInfo->SeparatorCaption = "audio out";
		}
	}



	// speed
	else if (ParamIndex >= (numOfAudiotInsOuts + 1) && ParamIndex < (1 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = sdkGetAudioQueryChannelNames("speed ", ParamIndex - numOfAudiotInsOuts - 1, queryIndex);
		pParamInfo->MinValue = -1.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->DefaultValue = 0.0f;
		//pParamInfo->Symbol = "s";
		pParamInfo->Format = "%.5f";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (1 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptDataFader;
		pParamInfo->Caption = "tape length";
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 120.0f;
		pParamInfo->DefaultValue = 0.0f;
		//pParamInfo->Symbol = "s";
		pParamInfo->Format = "%.5f";
		pParamInfo->Scale = scLinear;
		pParamInfo->IsInput = TRUE;
		pParamInfo->IsOutput = FALSE;
		pParamInfo->CallBackType = ctImmediate;
	}
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 2))
	{
	pParamInfo->ParamType = ptArray;
	pParamInfo->Caption = "set heads pos";
	pParamInfo->IsInput = true;
	pParamInfo->IsOutput = false;
	pParamInfo->ReadOnly = false;
	pParamInfo->MinValue = 0.0f;
	pParamInfo->MaxValue = 1.0f;
	pParamInfo->CallBackType = ctImmediate;
}
	else if (ParamIndex == (3 + numOfAudiotInsOuts * 2))
	{
		pParamInfo->ParamType = ptArray;
		pParamInfo->Caption = "get heads pos";
		pParamInfo->IsInput = false;
		pParamInfo->IsOutput = true;
		pParamInfo->ReadOnly = true;
		pParamInfo->MinValue = 0.0f;
		pParamInfo->MaxValue = 1.0f;
		pParamInfo->CallBackType = ctImmediate;
	}

	else{}
}

//-----------------------------------------------------------------------------
// set the parameters events address
void filter::onSetEventAddress(int ParamIndex, UsineEventPtr pEvent)
{
	// audioInputs
	if (ParamIndex == 0)
	{
		audioInputs = pEvent;
	}
	// audioOutputs
	else if (ParamIndex < (numOfAudiotInsOuts + 1))
	{
		audioOutputs[ParamIndex - 1] = pEvent;
	}
	// speed
	else if (ParamIndex >= (numOfAudiotInsOuts + 1) && ParamIndex < (1 + numOfAudiotInsOuts * 2))
	{
		speed[ParamIndex - numOfAudiotInsOuts - 1] = pEvent;
	}
	else if (ParamIndex == (1 + numOfAudiotInsOuts * 2))
	{
		length = pEvent;
	}
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 2))
	{
		positions = pEvent;
	}
	else if (ParamIndex == (3 + numOfAudiotInsOuts * 2))
	{
		getPositions = pEvent;
	}
	else {}
	/*
	else if (ParamIndex == (2 + numOfAudiotInsOuts * 2))
	{
		speed2 = pEvent;
	}
	*/
}

//-----------------------------------------------------------------------------
// Parameters callback
void filter::onCallBack(UsineMessage *Message)
{
	
	// filter only message specific to this user module
	if (Message->message == NOTIFY_MSG_USINE_CALLBACK)
	{
		// Message->wParam is equal to ParamIndex
		if ((Message->wParam >= (numOfAudiotInsOuts + 1) && Message->wParam < (1 + numOfAudiotInsOuts * 2)) && (Message->lParam == MSG_CHANGE))
		{
			m_speed[Message->wParam - numOfAudiotInsOuts - 1] = sdkGetEvtData(speed[Message->wParam - numOfAudiotInsOuts - 1]);
			//sdkTracePrecision(m_speed[Message->wParam - numOfAudiotInsOuts - 1]);
		}
		
		else if ( Message->wParam == (1 + numOfAudiotInsOuts * 2) && (Message->lParam == MSG_CHANGE))
		{
			//lineSize = sdkGetEvtData(length);
			m_size = int(sdkGetEvtData(length) * sdkGetSampleRate());
			//TPrecision m_sizeTP = sdkGetEvtData(length) * sdkGetSampleRate();
			//sdkTracePrecision(m_sizeTP);
			if (m_size < blocSize) { m_size = blocSize; }
			else if (m_size > max_size) { m_size = max_size - 1; }
			else {}
		}
		else if (Message->wParam == (2 + numOfAudiotInsOuts * 2) && (Message->lParam == MSG_CHANGE))
		{
			int i;
			int length = sdkGetEvtSize(positions);
			
			for (i = 0; i < length; i++)
			{
				m_positions[i] = sdkGetEvtArrayData(positions, i);
			}
		}
		else {}

	}
}

void filter::onProcess()
{
//write the whole bloc in the buffer
	for (int j = 0; j < sdkGetEvtSize(audioInputs); j++)
	{
		WriteSample(sdkGetEvtArrayData(audioInputs, j));
	}

	//m_write = m_write - sdkGetEvtSize(audioInputs);

	for (int i = 0; i < numOfAudiotInsOuts; i++) {
		//m_delayInt[i] = m_delay[i] *sdkGetSampleRate();

		sdkCopyEvt(audioInputs, audioOutputs[i]);
		int cursor = m_write - sdkGetEvtSize(audioInputs);
		for (int j = 0; j < sdkGetEvtSize(audioOutputs[i]); j++)
		{			
			m_delaySmoothed[i] = m_delaySmoothed[i] - (5.0f * m_speed[i]-1);
			
			//if (m_delaySmoothed[i] > m_delayInt[i]+1) { m_delaySmoothed[i] -=  (m_speed[i] - 1); } // --->
			//else if (m_delaySmoothed[i] < m_delayInt[i]-1) { m_delaySmoothed[i] += (m_speed2 + 1); } // <---

			if (m_delaySmoothed[i] > m_size) { m_delaySmoothed[i] = 0.0f; }
			else if (m_delaySmoothed[i] < 0) { m_delaySmoothed[i] = (float)m_size; }

			m_delayFinal[i] = m_delaySmoothed[i] + m_positions[i]* m_size;

			//sdkTracePrecision(m_delaySmoothed[i]);

			TPrecision tapout = 0.8f * GetDelayedSample((int)m_delayFinal[i], cursor);
			
			if (tapout - tapOutOld[i] > 0.8f) {
				//tapout = tapout + 0.5f;
				tapout = (tapout + tapOutOld[i]) / 2;
			}
			else if(tapout - tapOutOld[i] < 0.8f) {
				//tapout = tapout - 0.5f;
				tapout = (tapout + tapOutOld[i]) / 2;
			}
			//tapout = tapout + (tapOutOld[i] - tapout)* 0.9f; // passe bas
			tapOutOld[i] = tapout;

			sdkSetEvtArrayData(audioOutputs[i], j, tapout);
			cursor++;
		}
		sdkSetEvtArrayData(getPositions, i, ((TPrecision)(((int)m_delayFinal[i]) % m_size) / m_size));
	}
	
}

/*
float filter::map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/

// audio setup update
void filter::onBlocSizeChange(int BlocSize) {}
void filter::onSampleRateChange(double SampleRate) {}


/////////////////////////

//Writes a sample into buffer
void filter::WriteSample(
	TPrecision sample	// Sample Value
	)
{
	//write sample
	m_buffer[m_write] = sample;
	//update write position
	m_write = (m_write + 1) % m_size;
}
/*
//Reads a sample from buffer
TPrecision filter::ReadSample(int i)
{
	TPrecision retVal;

	//read sample
	retVal = m_buffer[m_read[i]];
	//update read position
	m_read[i] = (m_read[i] + 1) % m_size;
	return retVal;
}
*/
/*
//Process a given sample 
void filter::Process(
	TPrecision* sample		// Address to store processed sample value
	)
{
	//Write sample into delay's buffer
	WriteSample(*sample);

	//Update current value of sample with delayed value
	*sample = ReadSample();
}
*/
//Reads a delayed sample from buffer
TPrecision filter::GetDelayedSample(
	int delay, int cursor			// Delay expressed in samples
	)
{
	int sampleIndex = (cursor - delay -1) % m_size;
	//sdkTraceInt(sampleIndex);
	return sampleIndex >= 0 ? m_buffer[sampleIndex] : m_buffer[m_size + sampleIndex];
}
