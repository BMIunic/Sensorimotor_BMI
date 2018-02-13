/*************************************************************
/IMAGER_v2.0 (2016)
/nsp.h
/Decleration of function used in nsp.cpp
/Author: Aamir Abbasi
**************************************************************/

#ifndef NSP_H
#define NSP_H

#include <cbsdk.h>
cbSdkResult NSP_AnalogOut(cbSdkWaveformData wf, UINT16 channel);
cbSdkResult NSP_DigitalOut(UINT16 channel, UINT16 value);
cbSdkResult  NSP_AnalogOut_Disable(cbSdkWaveformData wf, UINT16 channel);
int NSP_On_Open();
int NSP_On_Close();
cbSdkWaveformData waveform(int repsAnalogPulse, UINT16 nDuration[], INT16 nAmplitude[], int nPhases);
cbSdkWaveformData instant_waveform(int repsAnalogPulse, UINT16 nDuration[], INT16 nAmplitude[], int nPhases);
cbSdkWaveformData disable_waveform(int, UINT16, INT16, int);
cbSdkWaveformData sine_waveform(int repsAnalogPulse, UINT16 nFrequency, INT16 nAmplitude);

#endif // NSP_H
