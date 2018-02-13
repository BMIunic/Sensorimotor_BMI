//////////////////////////////////////////////////////////////////////////////
///
/// @file shulz_extension.c
/// @author  Hyrum L. Sessions
/// @copyright (c) Copyright Blackrock Microsystems, LLC
/// @brief
///    Send a log packet on either a spike event or a digital input event
/// @details
///    Tested with C99, needs stdint support.
///
/// @date 21 Dec 2017
///
//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1 ///< define true for C
#endif

#ifndef FALSE
#define FALSE 0 ///< define false for C
#endif

#include "../ExtensionCommon/nspPlugin.h"
#include "../ExtensionCommon/nspChanTrigPlugin.h"

#define REQSAMPLES 10
#define REQSPIKES 64

cbExtChanTrigSamples samples;
uint16_t diginp[CBEXT_DIGITALINPUT_COUNT][REQSAMPLES];
uint16_t diginLast;
cbExtChanTrigOutputSamples SampleArray;
float anainp[CBEXT_INPUT_COUNT][REQSAMPLES] = {{0}};
cbExtChanTrigSpikes spikes;
cbExtSpikeElement spikeElement[REQSPIKES];

uint16_t g_nSpikeChan = 0;   ///< channel number to look for spikes (0=all)
uint16_t g_nSpikeUnit = 0;   ///< unit number to look for (0=all)
	
int InitFlag = 0; ///< to set the start and stop of streaming spikes
//int test = 0;

/// @author	Hyrum L. Sessions
/// @date	21 Dec 2017
/// @brief	Read comments from Central to set the channel number or unit number to monitor
void ProcessComments(cbExtComment cmt)
{
    cbExtResult res = CBEXTRESULT_SUCCESS;
	char logMsg[128]; // Message buffer
	memset(logMsg, 0, sizeof(logMsg));

    if (0 < strlen(cmt.szCmt))
    {
        if (strstr(cmt.szCmt, "Chan"))       // Chan=#  Sets the channel number to monitor (0=all)
        {
			char *posEqual = strstr(cmt.szCmt, "=");
			g_nSpikeChan = atoi(&posEqual[1]);
			sprintf(logMsg, "Chan set to %d\n", g_nSpikeChan);
			res = cbExtLogEvent(logMsg);
        }
        else if (strstr(cmt.szCmt, "Unit"))       // Unit=#  Sets the unit number to monitor (0=all)
        {
			char *posEqual = strstr(cmt.szCmt, "=");
			g_nSpikeUnit = atoi(&posEqual[1]);
			sprintf(logMsg, "Unit set to %d\n", g_nSpikeUnit);
			res = cbExtLogEvent(logMsg);
        }
		else if (strstr(cmt.szCmt, "GetPackets"))       // Start streaming spikes
        {
			char *posEqual = strstr(cmt.szCmt, "=");
			InitFlag=1;
			sprintf(logMsg, "Start streaming spikes %d\n",InitFlag);
        }
		else if (strstr(cmt.szCmt, "Stop"))       // Stop streaming spikes
        {
			char *posEqual = strstr(cmt.szCmt, "=");
			InitFlag=0;
			sprintf(logMsg, "Stop streaming spikes %d\n",InitFlag);
        }
        else                                        // unknown comment so we'll just display it
        {
            sprintf(logMsg, "Comment '%s' charset %u data %u flags %u\n", cmt.szCmt, cmt.nCharset, cmt.nData, cmt.nFlag);
            res = cbExtLogEvent(logMsg);
        }
    }
} // end if (res ==

/// @author	Hyrum L. Sessions
/// @date	21 Dec 2017
/// @brief	Read comments from Central to set the channel number or unit number to monitor
void ProcessAnalogInputs(void)
{
	if (InitFlag!=0) 
	{
		cbExtResult res = CBEXTRESULT_SUCCESS;
		int nIndex;
		uint32_t procTime;
		char logMsg[128]; // Message buffer
		memset(logMsg, 0, sizeof(logMsg));

		// let's invert the signal on ain
		if (0 < samples.isCount.nCountAnalogInput)
		{
			res = cbExtGetCurrentSampleTime(&procTime);
			for (nIndex = 0; nIndex < samples.isCount.nCountAnalogInput; ++nIndex)
			{
				if (diginLast != diginp[0][nIndex])
				{
					sprintf(logMsg, "T:%d D:%d\n", procTime, diginp[0][nIndex]);
					cbExtLogEvent(logMsg);
					diginLast = diginp[0][nIndex];
				}
			}
		}
	}
}

/// @author Hyrum L. Sessions
/// @date   21 Dec 2017
/// @brief  A spike has been found so let's send a log packet
void ProcessSpike(void)
{
	if (InitFlag!=0) 
	{
		char logMsg[1024];
		int nIndex;

		for (nIndex = 0; nIndex < spikes.isCount.nCountSpikes; ++nIndex)
		{
			if (32 > spikes.isSpike[nIndex].nChan)
			{
				if ((g_nSpikeChan == 0) || (g_nSpikeChan == spikes.isSpike[nIndex].nChan))
				{
					if ((g_nSpikeUnit == 0) || (g_nSpikeUnit == spikes.isSpike[nIndex].nUnit))
					{
						sprintf(logMsg, "T:%d C:%d U:%d\n", spikes.isSpike[nIndex].nTime, spikes.isSpike[nIndex].nChan, spikes.isSpike[nIndex].nUnit);
						cbExtLogEvent(logMsg);
					}
				}
			}
		}
	}
}

/// @author Hyrum L. Sessions
/// @date   21 Dec 2017
/// @brief  main plugin loop
cbExtResult cbExtMainLoop(cbExtSettings * settings)
{
    int nIndex;
    cbExtResult res = CBEXTRESULT_SUCCESS;

    // initialize the buffers
    memset(diginp, 0, sizeof(diginp));
    memset(anainp, 0, sizeof(anainp));
    memset(&samples, 0, sizeof(samples));
	char logMsg[128]; // Message buffer
	memset(logMsg, 0, sizeof(logMsg));
	
	// prepare the digital input structure
    for (nIndex = 0; nIndex < CBEXT_DIGITALINPUT_COUNT; ++nIndex)
    {
        samples.isDigitalInput.pnData[nIndex] = &diginp[nIndex][0];
    }
    samples.isCount.nCountDigitalInput = REQSAMPLES;
    // prepare front end / analog input structures
    for (nIndex = 0; nIndex < CBEXT_INPUT_COUNT; ++nIndex)
    {
        if (nIndex < CBEXT_FRONTEND_COUNT)
            samples.isFrontend.pfData[nIndex] = &anainp[nIndex][0];
        else
            samples.isAnalogInput.pfData[nIndex-CBEXT_FRONTEND_COUNT] = &anainp[nIndex][0];
    }
    samples.isCount.nCountAnalogInput = REQSAMPLES;
    // prepare spike input structure
    spikes.isSpike = &spikeElement[0];
    spikes.isCount.nCountSpikes = REQSPIKES;

    // This loop determines the life-span of the plugin,
    //  it is the plugin that should empty its buffers on time, or plugin may loose data
    while (res != CBEXTRESULT_EXIT)
    {
        // Check if comment packet need to be processed
        cbExtComment cmt;
        res = cbExtGetOneComment(&cmt);
        if (res == CBEXTRESULT_EXIT)                        // error so we'll exit
        {
            sprintf(logMsg, "Error getting a comment\n");
            cbExtLogEvent(logMsg);
            break;
        }
        if (res == CBEXTRESULT_SUCCESS)
        {
            ProcessComments(cmt);
        }

        // now we'll get front end input data if requested
        // now we'll get analog input data if requested
        samples.isCount.nCountFrontend = REQSAMPLES;
        samples.isCount.nCountAnalogInput = REQSAMPLES;
        samples.isCount.nCountDigitalInput = REQSAMPLES;
        res = cbExtChanTrigGetSamples(&samples);
        if (res == CBEXTRESULT_EXIT)                        // error so we'll exit
        {
            sprintf(logMsg, "Error getting samples\n");
            cbExtLogEvent(logMsg);
            break;
        }
        if (res == CBEXTRESULT_SUCCESS)
        {
            ProcessAnalogInputs();
        }
        else
        {
            printf("some error occured %d\n", res);
        }
        
        // now we'll get spike data if requested
        memset(spikeElement, 0, sizeof(spikeElement));
        spikes.isCount.nCountSpikes = REQSPIKES;
        res = cbExtChanTrigGetSpikes(&spikes);
        if (res == CBEXTRESULT_EXIT)                        // error so we'll exit
        {
            sprintf(logMsg, "Error getting spikes\n");
            cbExtLogEvent(logMsg);
            break;
        }
        else if (res == CBEXTRESULT_SUCCESS)
        {
            ProcessSpike();
        }
        else
        {
            printf("some spike error occured %d\n", res);
        }
    } // end while (res != CBEXTRESULT_EXIT
    sprintf(logMsg, "Goodbye!\n");
	cbExtLogEvent(logMsg);
    return CBEXTRESULT_SUCCESS;
}

/// @author Hyrum L. Sessions
/// @date   21 Dec 2017
/// @brief  Extension setup<br>
cbExtResult cbExtSetup(cbExtInfo * info)
{
    // The pieces in info that are not set here will have 0 as default, which disables them
    info->nPluginVer = 1; // Give this a version
    info->nWarnCommentsThreshold = 90; // Warn on 90% buffer
    strncpy(info->szName, "Shulz Extension", sizeof(info->szName));
    cbExtCommentMask iMask;
    iMask.nCharsetMask = 0x90;    // Interested in charsets of 128 and 16 (0x80 + 0x10)
    iMask.flags = CBEXT_CMT_NONE; // also want normal comments, but am not interested in NeuroMotive events
    info->iMask = iMask;

    return CBEXTRESULT_SUCCESS;
}

/// @author Hyrum L. Sessions
/// @date   21 Dec 2017
/// @brief  This plugin was written for the Shulz lab to send a comment on a spike or digital event
cbExtResult cbExtChanTrigSetup(cbExtChanTrigInfo * info)
{
    // Having this function defined is equivalant of chantrig intent
    // The pieces in info that are not set here will have 0 as default, which disables them

    info->nDividerDigitalInput = 1; // Need digital input channel at full sample-rate
    info->nDividerFrontend = 1;     // Need front end full sample-rate
    info->nDividerAnalogInput = 1;  // Need analog input full sample-rate
    info->nSpikes = 1;
    return CBEXTRESULT_SUCCESS;
}
