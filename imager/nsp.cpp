/*************************************************************
/IMAGER_v2.0 (2016)
/nsp.cpp
/cbSdk.h based functions for generating analog waveforms
/Author: Aamir Abbasi
**************************************************************/
#include "cbsdk.h"
#include "nsp.h"


using namespace std;

UINT32 nInstance = 0;
cbSdkResult  NSP_AnalogOut(cbSdkWaveformData wf, UINT16 channel)
{
    // Generate analog waveform
    cbSdkResult res = cbSdkSetAnalogOutput(nInstance, channel, &wf, NULL);
    return res;
}

cbSdkResult NSP_DigitalOut(UINT16 channel, UINT16 value)
{
    // Set digital output high or low
    cbSdkResult res = cbSdkSetDigitalOutput(nInstance, channel, value);
    return res;
}

cbSdkResult  NSP_AnalogOut_Disable(cbSdkWaveformData wf, UINT16 channel)
{
    // Generate analog waveform
    cbSdkResult res = cbSdkSetAnalogOutput(nInstance, channel, &wf, NULL);
    return res;
}

int NSP_On_Open()
{
    // Open the interface with Blackrock NSP
    cbSdkConnection con;
    cbSdkConnectionType conType = CBSDKCONNECTION_UDP;
    cbSdkResult res = cbSdkOpen(nInstance, conType, con);
    if (res != 0){
        return 1;
    }
    return 0;
}

int NSP_On_Close()
{
    // Close the interface with Blackrock NSP
    cbSdkResult res = cbSdkClose(nInstance);
    if (res != 0){
        return 1;
    }
    return 0;
}

cbSdkWaveformData waveform(int repsAnalogPulse, UINT16 nDuration[], INT16 nAmplitude[], int nPhases)
{
    // Define the analog waveform
	cbSdkWaveformData wf;
	wf.type = cbSdkWaveform_PARAMETERS;
	wf.repeats = repsAnalogPulse;
	wf.trig = cbSdkWaveformTrigger_DINPREG;  //cbSdkWaveformTrigger_NONE
    wf.trigChan = 1;
	wf.trigValue = 0;
	wf.trigNum = 0;
	wf.offset = 0;
	wf.duration = nDuration;
	wf.amplitude = nAmplitude;
	wf.phases = nPhases;
    return wf;
}


cbSdkWaveformData instant_waveform(int repsAnalogPulse, UINT16 nDuration[], INT16 nAmplitude[], int nPhases)
{
    // Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS;
    wf.repeats = repsAnalogPulse;
    wf.trig = cbSdkWaveformTrigger_NONE;
    wf.trigChan = 0;
    wf.trigValue = 0;
    wf.trigNum = 0;
    wf.offset = 0;
    wf.duration = nDuration;
    wf.amplitude = nAmplitude;
    wf.phases = nPhases;
    return wf;
}

cbSdkWaveformData disable_waveform(int , UINT16 , INT16 , int )
{
    // Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_NONE;
    wf.repeats = NULL;
    wf.trig = cbSdkWaveformTrigger_NONE;  //cbSdkWaveformTrigger_NONE
    wf.trigChan = 0;
    wf.trigValue = 0;
    wf.trigNum = 0;
    wf.offset = 0;
    wf.duration = NULL;
    wf.amplitude = NULL;
    wf.phases = NULL;
    return wf;
}

cbSdkWaveformData sine_waveform(int repsAnalogPulse, UINT16 nFrequency, INT16 nAmplitude){

    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_SINE;
    wf.repeats = repsAnalogPulse;
    wf.trig = cbSdkWaveformTrigger_NONE;  //cbSdkWaveformTrigger_NONE
    wf.trigChan = 0;
    wf.trigValue = 0;
    wf.trigNum = 0;
    wf.offset = 0;
    wf.sineAmplitude = nAmplitude;
    wf.sineFrequency = nFrequency;
    return wf;
}
