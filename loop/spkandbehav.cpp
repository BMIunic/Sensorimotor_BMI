//////////////////////////////////////////////////////////////////////////////
/// Sensorimotor BMI GUI (2016)
/// This file containes the definations of routines to read spikes and control
/// projector and rewards
#include "spkandbehav.h"

std::default_random_engine generator;
std::exponential_distribution<double> update_target(2000);
std::uniform_real_distribution<double> sin_amp;
std::uniform_real_distribution<double> sin_phase;
std::uniform_real_distribution<double> sin_freq;

#if DEMO == 1
std::poisson_distribution<int> distribution(0.5);
std::poisson_distribution<int> lick_distribution(0.01);
#endif

/// Define statics here...
QList<unsigned int> SpkAndBehav::SpkList;
QList<unsigned int> SpkAndBehav::LickList;

/// Defination of local variables
int SpkAndBehav::currentLickDIN;

SpkAndBehav::SpkAndBehav(QObject *parent) :
    QObject(parent)
{
#if DEMO == 0
    cbSdkConnection con;
    cbSdkConnectionType conType = CBSDKCONNECTION_UDP;
    res = cbSdkOpen(0, conType, con);
#endif

    paramMut.lock();
    currentDataFolder = dataFolder;
    currentCameraAOUT = cameraAOUT;
    currentWaterAOUT = waterAOUT;
    currentLickDIN = lickDIN;
    paramMut.unlock();

    /// Set digital in for the licking input
    LickHist = 0;

    /// Seed the random generator
    res = cbSdkGetTime(0, &CurrentTime);
    qsrand(CurrentTime);

    /// Session was not active before
    ActiveBefore = 1;

    m_timer = new QTimer(this);
    m_timer->setInterval(RoutineTime); // in milliseconds
    m_timer->start();

    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(routine()));

#if DEMO == 0
    /// Tell pluging to start delivering spikes/digital packets
    res = cbSdkSetComment(0, 0, 0, "GetPackets");

    /// Register the callback
    res = cbSdkRegisterCallback(0, CBSDKCALLBACK_LOG, LogCallback, 0);
#endif

#if DEMO == 1
    s_timer = new QTimer(this);
    s_timer->setInterval(1000); // <<ms
    s_timer->start();
    QObject::connect(s_timer, SIGNAL(timeout()), this, SLOT(add_fake_spike()));
#endif
}

void SpkAndBehav::closeProgram()
{
    /// Disconnect the perpetual call to the routine
    QObject::disconnect(m_timer, SIGNAL(timeout()), this, SLOT(routine()));

#if DEMO == 0
    /// Unregister the callback
    res = cbSdkUnRegisterCallback(0, CBSDKCALLBACK_LOG);

    /// Plugin stop sending spikes
    cbSdkSetComment(0, 0, 0, "Stop");
#endif
}


#if DEMO == 0
/// Callback routine: Read all the packets!
void SpkAndBehav::LogCallback(UINT32, const cbSdkPktType type, const void* pEventData, void* pCallbackData)
{
    char *nPos;
    int nTime;
    int nChan;
    int nUnit;
    int nDigValue;

    // Note: Callback should return fast!
    switch(type)
    {
    case cbSdkPkt_LOG:
        if (pEventData)
        {
            cbPKT_LOG pLog = *reinterpret_cast<const cbPKT_LOG *>(pEventData);
            // initialize values
            nTime = nChan = nUnit = nDigValue = 0;
            // get the timestamp
            nPos = strstr(pLog.desc, "T:");
            if (nPos)
                nTime = atoi(&nPos[2]);
            // check for digital entry
            nPos = strstr(pLog.desc, "D:");
            if (nPos)
                nDigValue = atoi(&nPos[2]);
            // check for channel
            nPos = strstr(pLog.desc, "C:");
            if (nPos)
                nChan = atoi(&nPos[2]);
            // check for unit
            nPos = strstr(pLog.desc, "U:");
            if (nPos)
                nUnit = atoi(&nPos[2]);

            if (nChan){
                // Get spike activity in a QList
                //printf("T:%d C:%d U:%d\n", nTime, nChan, nUnit);
                SpkList.append(nTime);
                i++;
                printf("compteur: %d" ,i);
                printf("  ");
            }
            else{
                // Get lick activity in a QList
                //printf("T:%d D:%d\n", nTime, nDigValue);
                LickList.append(nTime);}
        }
        break;
    default:
        break;
    }
    return;
}
#endif

#if DEMO ==  1
void SpkAndBehav::add_fake_spike()
{
    SpkList.append((QDateTime::currentDateTime().toTime_t()-1480000000)*36000);
}
#endif

/// Main routing begins here!!!
void SpkAndBehav::routine()
{
    /// First, check if session starts or stops
    if ((ActiveBefore == 1) && (Active.load() >= 2) && (Active.load() < 4)) // session starts
    {
        initSession(); // start session

        if (Active.load() == 3)
        {
            start_cameraTrig();  // start camera trigger
        }
        ActiveBefore = Active.load();
    }
    if ((ActiveBefore == 1) && (Active.load()== 4)) // control session starts
    {
        initControlSession(); // start session
        counter=0;
        ActiveBefore = Active.load();
    }
    if ((ActiveBefore >= 2) && (Active.load() < 2)) // session stops
    {
        FrameIdx_bfr_before = -1;
        if (Active.load() == 1){
            stop_cameraTrig(); // stop camera trigger
            stopSession(); // stop the recording
            counter=0;
            ActiveBefore = 1;
        } else {
            stop_cameraTrig();  // stop camera trigger
            stopSession(); // stop session
            closeProgram(); // stop recording
            ActiveBefore = 0;
        }
    }

    /// Get current NSP time
#if DEMO == 0
    res = cbSdkGetTime(0, &CurrentTime); // in fractions of the 30000 Hz clock
#endif

#if DEMO == 1
    CurrentTime = (QDateTime::currentDateTime().toTime_t()-1480000000)*36000;
#endif

    if (MeanFR_active.load() == 0){
        // if the spike list grows more than maxSpkCount: remove the oldest spikes
        while (SpkList.count()> maxSpkCount)
        {
            SpkList.removeFirst();
        }
        young_spk_count = 0;
        for (int i = (SpkList.count()-1); i>=0 ; i--)
        {
            if (static_cast<int>(CurrentTime - SpkList[i]) > (binsizeFR*(30000/1000))) // Sampling rate = 30000 Hz
            { // too old, stop counting
                break;
            }
            young_spk_count += 1;
        }
        SpkCount.store(young_spk_count*(1000/binsizeFR));
    }

    /// waiting period starts here to calculate the mean firing rate!!!
    meanFR_active_flag = 0;
    if (MeanFR_active.load() == 1){

        // Get out of waiting period after the prespecified period
        if (((float)(std::clock()/CLOCKS_PER_SEC) - MeanFR_StartTime) == (WaitingPRD.load()*60)){

            MeanFR_active.store(0);
            meanFR_active_flag = 1;
            cbSdkSetComment(0, 0, 0, "waiting ends");
        }

        cbSdkGetTime(0, &CurrentTime); // in fractions of the 30000 Hz clock

        // if the spike list grows more than 2000: remove the oldest spikes
        while (SpkList.count()> maxSpkCount)
        {
            SpkList.removeFirst();
        }
        young_spk_count = 0;
        for (int i = (SpkList.count()-1); i>=0 ; i--)
        {
            if (static_cast<int>(CurrentTime - SpkList[i]) > (binsizeFR*(30000/1000))) // Sampling rate = 30000 Hz
            { // too old, stop counting
                break;
            }
            young_spk_count += 1;
        }

        SpkCount.store(young_spk_count*(1000/binsizeFR));
        spkcount_vector.append(young_spk_count*(1000/binsizeFR));

    }

    /// Compute the mean and sd of the firing rate
    if (MeanFR_active.load() == 0 && meanFR_active_flag == 1)
    {
        uint meanSUM = 0;
        for (int i = 0; i < spkcount_vector.length(); i++){
            meanSUM += spkcount_vector[i];
        }
        MeanFR = (float)meanSUM / (float)spkcount_vector.length();
        for (int i = 0; i< spkcount_vector.length(); i++){
            SdFR += qPow((spkcount_vector[i] - MeanFR),2);
        }
        SdFR /= (spkcount_vector.length() -1);
        SdFR = qSqrt(SdFR);
        spkcount_vector.clear();
    }

    ////////////////////////////////////////////
    /// Check the lick activity
    if (LickList.count()>0) //if chid is 151 the lick are correctly read then this condition is satisfied
    {
        int OldLickTime = LickList[0];
        while (static_cast<int>(CurrentTime - OldLickTime) > (binsizeFR*(30000/1000)))
        {
            LickList.removeFirst();
            if (!LickList.isEmpty())
            {
                OldLickTime = LickList[0];
            }
            else
            {
                break;
            }
        }
        LastLick.store(LickList.last());
    }
    else
    {
        LastLick.store(0);
    }

    /// If the session is running in closed loop mode
    /// with high speed camera Active.load ==3
    /// without camera Active.load == 2
    if (Active.load() == 2 || Active.load() == 3)
    {

        /// Automatically stop the session after the SessionPRD
        if (((float)(std::clock()/CLOCKS_PER_SEC) - Session_StartTime) == (SessionPRD.load()*60)){
            // turn off the session, not the whole GUI
            Active.store(1);
        }

        uint32_t FrameIdx_bfr;

        if ((UpperThd.load() - LowerThd.load()) > 0)
        {
            /// Link between spike count and feedback frame sequence.
            FrameIdx_bfr = (uint32_t) (FrameList.count()-1)*(((double)SpkCount.load() - LowerThd.load())/(UpperThd.load() - LowerThd.load()));
        }


        FrameIdx_bfr = qMin(FrameIdx_bfr, (uint32_t)(FrameList.count() - 1));
        FrameIdx_bfr = qMax(FrameIdx_bfr, (uint32_t)0);
        FrameIdx.store(FrameIdx_bfr);

#if DEMO == 0
        /// Projector
        /// Stream and dispaly the current sequence on the projector
        Flip.store((Flip.load()+1)%2); // flip goes 0 1 0...  ///TO BE TESTED
        for (int j = 0; j < frame_count; j++)
        {
            AlpSeqPut( nDevId, nSeqId_list[Flip.load()], j, 1, FrameList[FrameIdx.load()][j].bits());
        }
        AlpProjStart(nDevId, nSeqId_list[Flip.load()]); // append single frame sequence to the stack

        /// Log the sequence id as NSP comments
        if (FrameIdx_bfr_before != FrameIdx_bfr) {
            res = cbSdkSetComment(0, 0, 0, ("F:" + QString::number(FrameIdx_bfr)).toStdString().c_str());
            FrameIdx_bfr_before = FrameIdx_bfr;
        }
#endif
        /// Reward
        /// Identify a reward area, and if the firing rate holds in the area for long enough, trigger a reward sequence
        if ((FrameIdx_bfr >= RewardLowerThd.load()) && (FrameIdx_bfr < RewardUpperThd.load()))
        {
            if (LastLick.load() > 0 && LastLick.load() - LickHist != 0 && Water.load() == 0) {
                if (rewardCounter < rewardThd){ // rewardInterval = 0 unlimited reward as long as the firing rate hold within the range. rewardThe != 0 limited reward.
                    res = cbSdkSetComment(0, 0, 0, "OneWaterReward");
                    one_waterReward();
                    rewardCounter++;
                    LastReward.store(1);
                    RewardCounter.store(RewardCounter.load() + rewardCounter); // Store total number of rewards
                }
                LickHist = LastLick.load();
            }
        }
        else
        {
            rewardCounter = 0;
        }
    }

    /// If the session is running in open loop mode
    if (Active.load() == 4) // session for control experiemnts (acute recordings in S1)
    {

        /// Condition to clock the session duration and automatically stop the session when session duration reaches the value in SessionPRD parameter
        if (((float)(std::clock()/CLOCKS_PER_SEC) - Session_StartTime) == (SessionPRD.load()*60)){
            // turn off the session, not the whole GUI
            Active.store(1);
        }

        uint32_t FrameIdx_bfr;

        SpkCount.store(BufferList.at(counter));
        if (counter+1 == BufferList.length()){ // input file is finished stop the session
            Active.store(1);
            counter = 0;
            BufferList.clear();
        }
        FrameIdx_bfr = BufferList.at(counter);
        counter +=1;

        FrameIdx_bfr = qMin(FrameIdx_bfr, (uint32_t)(FrameList.count() - 1));
        FrameIdx_bfr = qMax(FrameIdx_bfr, (uint32_t)0);
        FrameIdx.store(FrameIdx_bfr);

        //Switch to next sequence
        AlpProjStart(nDevId, nSeqId_list[FrameIdx_bfr]);

        // log frame numbers as comments

        if (FrameIdx_bfr_before != FrameIdx_bfr) {
            res = cbSdkSetComment(0, 0, 0, ("F:" + QString::number(FrameIdx_bfr)).toStdString().c_str());
            FrameIdx_bfr_before = FrameIdx_bfr;
        }
    }

    /// Water flow activation/stop
    if ((Water.load() == 1) && !(WaterBefore == 1))
    {
        WaterBefore = Water.load();
        //qDebug() << "continous water reward";
        continuous_waterReward();
    }
    if ((Water.load() == 0) && !(WaterBefore == 0))
    {
        WaterBefore = Water.load();
        stop_water();
    }
    if ((Water.load() == 2) && !(WaterBefore == 2))
    {
        WaterBefore = Water.load();
        start_flush();
    }
    if (Water.load() == 3)
    {
        res = cbSdkSetComment(0, 0, 0, "OneFreeReward");
        onefree_waterReward();
        Water.store(0);
        WaterBefore = Water.load();
    }
}

void SpkAndBehav::initSession(){
    // #if DEMO == 0
    //    StartTime_s = QDateTime::currentDateTime().toTime_t();

    //    /// define the 30k continuous acquisition, no filter, of the data across all 32 channels
    //    cbPKT_CHANINFO chan_info;

    //    for (int i = 0; i < 32 ; i++){
    //        res = cbSdkGetChannelConfig(0, 1, &chan_info); //get current channel configuration
    //        //change configuration
    //        chan_info.smpfilter = 0; //continuous filter (none)
    //        chan_info.smpgroup = 5; //continuous sampling rate (30kHz)
    //        //set channel configuration
    //        res = cbSdkSetChannelConfig(0, i+1, &chan_info); //note: channels start at 1
    //    }

    //    /// Configure the trial to include both continuous and event data
    //    uint16_t bActive = 1; // reset the trial
    //    uint16_t uBegChan   = 1;
    //    uint32_t uBegMask   = 0;
    //    uint32_t uBegVal    = 0;
    //    uint16_t uEndChan   = 32;
    //    uint32_t uEndMask   = 0;
    //    uint32_t uEndVal    = 0;
    //    bool   bDouble    = false;
    //    bool   bAbsolute  = false;
    //    uint32_t uWaveforms = 0;
    //    uint32_t uConts     = cbSdk_CONTINUOUS_DATA_SAMPLES; // record the continuous data
    //    uint32_t uEvents    = cbSdk_EVENT_DATA_SAMPLES;
    //    uint32_t uComments  = 0; // should we record the comments ?
    //    uint32_t uTrackings = 0;

    //    res = cbSdkSetTrialConfig(0, bActive, uBegChan, uBegMask, uBegVal, uEndChan, uEndMask, uEndVal, bDouble, uWaveforms, uConts, uEvents, uComments, uTrackings, bAbsolute);
    //#endif
    //    /// Init the data recording: look at the next session number in the data folder of the current mouse
    //    /// create folder that will contain the data
    //    paramMut.lock();
    //    QString currentSessionFolderName = currentDataFolder + "mouse" + QString::number(Mouse.load()) + "/session" + QString::number(Session.load()) + '/';
    //    paramMut.unlock();
    //    /// create folder that will contain the data
    //    QDir dir;
    //    dir.mkdir(currentSessionFolderName);
    //    QString FileName = currentSessionFolderName + "data";
    //    /// Init the file.
    //    res = cbSdkSetFileConfig(0, FileName.toStdString().c_str(), "", true, cbFILECFG_OPT_NONE);

    //    /// Sleep for few seconds to allow launch of the recording
    //    Sleep(2000);
#if DEMO == 0
    /// init the recording
    res = cbSdkSetComment(0, 0, 0, "Init session");

    /// Log as events the mouse, session
    res = cbSdkSetComment(0, 0, 0, ("Mouse:"+ QString::number(Mouse.load())).toLocal8Bit().constData());
    res = cbSdkSetComment(0, 0, 0, ("Session:"+ QString::number(Session.load())).toLocal8Bit().constData());
    /// Log as events the date
    QDateTime time = QDateTime::currentDateTime();
    res = cbSdkSetComment(0, 0, 0, ("Date:"+ time.toString("dd/MM/yy")).toStdString().c_str()); // the date
    res = cbSdkSetComment(0, 0, 0, ("Hour:"+ time.toString("hh:mm:ss")).toStdString().c_str()); // the hour/minutes/seconds.

    /// Log as event protocol id
    if (RandProtocolID.load() == 0){
        cbSdkSetComment(0, 0, 0, ("ProtocolID:" + QString::number(RandProtocolID.load()) + " topy_0deg").toStdString().c_str());
    }
    if (RandProtocolID.load() == 1){
        cbSdkSetComment(0, 0, 0, ("ProtocolID:" + QString::number(RandProtocolID.load()) + " topy_90deg").toStdString().c_str());
    }
    if (RandProtocolID.load() == 2){
        cbSdkSetComment(0, 0, 0, ("ProtocolID:" + QString::number(RandProtocolID.load()) + " rand").toStdString().c_str());
    }
    if (RandProtocolID.load() == 3){
        cbSdkSetComment(0, 0, 0, ("ProtocolID:" + QString::number(RandProtocolID.load()) + " black").toStdString().c_str());
    }

    /// Log as event bin size for computing firing rate
    cbSdkSetComment(0, 0, 0, ("BinSize:" + QString::number(binsizeFR)).toStdString().c_str());

    /// Log as event waiting period and session period
    cbSdkSetComment(0, 0, 0, ("WaitingPRD(min):" + QString::number(WaitingPRD.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("SessionPRD(min):" + QString::number(SessionPRD.load())).toStdString().c_str());

    /// Log as events mean firing rate and standard deviation
    cbSdkSetComment(0, 0, 0, ("mF:" + QString::number(MeanFR)).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("sDF:" + QString::number(SdFR)).toStdString().c_str());

    /// Log as events upper and lower firing rate thresholds
    cbSdkSetComment(0, 0, 0, ("TL:" + QString::number(LowerThd.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("TU:" + QString::number(UpperThd.load())).toStdString().c_str());

    /// Read all the channels and units as a QString for logging as comments
    for (int i=0; i < Channel.length(); i++)
    {
        QString comment_channels_units;
        comment_channels_units.append(QString::number(Channel.at(i)));
        comment_channels_units.append("-");
        for (int j = 0; j < Unit.length(); j++)
        {
            comment_channels_units.append((QString::number(Unit.at(j))));
        }
        /// Log slected channel numbers as comments in the recorded .dat file
        res = cbSdkSetComment(0,0,0,("ChU:" + comment_channels_units).toStdString().c_str());
    }

#endif
    /// Update the parameters that are used
    paramMut.lock();
    currentParam = param;
    paramMut.unlock();

    /// Load the Parameters
    rewardStrength = currentParam.rewardStrength;
    RewardLowerThd.store(currentParam.rewardLower);
    RewardUpperThd.store(currentParam.rewardUpper);
    timeOut = currentParam.timeOut;
    rewardInterval = currentParam.rewardInterval;
    rewardThd = currentParam.rewardThd;
    input = currentParam.input;
    brightness = currentParam.brightness;
    frameDuration = currentParam.frameDuration;
    frameIllumination = currentParam.frameIllumination;
    FrameList = currentParam.FrameList;

    /// Log as events some parameters
    cbSdkSetComment(0, 0, 0, ("rwdFrameTL:" + QString::number(RewardLowerThd.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("rwdFrameTU:" + QString::number(RewardUpperThd.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number((RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number((RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load())).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("brightness:" + QString::number(brightness)).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("frameDuration:" + QString::number(frameDuration)).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("frameIllumination:" + QString::number(frameIllumination)).toStdString().c_str());
    if (rewardThd == 0){
        cbSdkSetComment(0, 0, 0, ("rewardInterval:" + QString::number(rewardInterval)).toStdString().c_str());
    }
    else{
        cbSdkSetComment(0, 0, 0, ("rewardThd:" + QString::number(rewardThd)).toStdString().c_str());
    }

#if DEMO == 0
    /// start the projector
    ///
    //const int seq_count = FrameList.count();
    frame_count = FrameList[0].count();

    nSeqId_list = QVector<ALP_ID>(2);

    /// Allocate the ALP high-speed device
    if (ALP_OK != AlpDevAlloc( ALP_DEFAULT, ALP_DEFAULT, &nDevId )){
        qDebug() << "AlpDevAlloc failed. Check projector";
        return;
    }

    /// queue mode...
    if (ALP_OK != AlpProjControl(nDevId, ALP_PROJ_QUEUE_MODE, ALP_PROJ_SEQUENCE_QUEUE)){
        qDebug()<< "AlpProjControl failed. Check projector";
        return;
    }

    /// Allocate two sequences, one to display, the second to upload in background
    for (int i = 0; i < 2 ; i++) {
        /// init a sequence
        AlpSeqAlloc( nDevId, 1, frame_count, &(nSeqId_list[i]));
        AlpSeqTiming( nDevId, nSeqId_list[i],frameIllumination, frameDuration, ALP_DEFAULT, ALP_DEFAULT, ALP_DEFAULT );
        AlpSeqControl(nDevId, nSeqId_list[i],ALP_BITNUM,1);
    }

    /// initialize LED
    AlpLedAlloc( nDevId, ALP_HLD_PT120TE_BLUE, NULL, &AlpLedId );
    AlpLedControl( nDevId, AlpLedId, ALP_LED_BRIGHTNESS, brightness);

    /// Main LOOP
    ///AlpProjStart( nDevId, nSeqId_list[0]); // init display, ideally a black frame

#endif
}

void SpkAndBehav::one_waterReward(){
    /// if the water reward is continuous
#if DEMO == 0
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[2];
    wDur[0] = 900; //at 30 000 Hz
    wDur[1] = 300; // 10 ms at 30 000 Hz, just to ensure that the pulse goes down after 10ms

    /// voltage of the phases of the waveform
    int16_t wAmp[2];
    wAmp[0] = 32767; // 5V TTL
    wAmp[1] = 0; // 0 V baseline

    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 1; // one reward
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 2; // yet another way to set the number of repeats of the waveform

    res = cbSdkSetAnalogOutput(0, currentWaterAOUT, &wf, NULL);
#else
    qDebug() << "Single water reward";
#endif
}

void SpkAndBehav::continuous_waterReward(){
    /// if the water reward is continuous
#if DEMO == 0
    // analog out channel for the reward
    //UINT16 channel = 146; //Analog output channel number on NSP (channel 2)
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[2];
    wDur[0] = 900; // 100 ms at 30 000 Hz
    wDur[1] = 300; // 10 ms at 30 000 Hz, just to ensure that the pulse goes down after 10ms

    /// voltage of the phases of the waveform
    int16_t wAmp[2];
    wAmp[0] = 32767; // 5V TTL
    wAmp[1] = 0; // 0 V baseline

    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 1; // continuous reward
    wf.trig = cbSdkWaveformTrigger_DINPREG;
    wf.trigChan = 2; // Channel trigger
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 2; // yet another way to set the number of repeats of the waveform

    //res = cbSdkSetComment(0, 0, 0, "Continuous Water reward");
    res = cbSdkSetAnalogOutput(0, currentWaterAOUT, &wf, NULL);
#else
    qDebug() << "Continuous water reward";
#endif
}

void SpkAndBehav::onefree_waterReward(){
#if DEMO == 0
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[2];
    wDur[0] = 900; // 250 ms at 30 000 Hz
    wDur[1] = 300; // 10 ms at 30 000 Hz, just to ensure that the puse goes down after 10ms

    /// voltage of the phases of the waveform
    int16_t wAmp[2];
    // wAmp[0] = 32767; // 5V TTL
    wAmp[0] = 32767; // 5V TTL
    wAmp[1] = 0; // 0 V baseline


    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 4; // continuous reward
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 1; // yet another way to set the number of repeats of the waveform

    res = cbSdkSetAnalogOutput(0, currentWaterAOUT, &wf, NULL);
#else
    qDebug() << "One free reward";
#endif
}

void SpkAndBehav::start_flush(){
#if DEMO == 0
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[2];
    //wDur[0] = 300; // 10 ms at 30 000 Hz. Constantly ON
    wDur[0] = 7000; // 250 ms at 30 000 Hz
    wDur[1] = 300; // 10 ms at 30 000 Hz, just to ensure that the puse goes down after 10ms

    /// voltage of the phases of the waveform
    int16_t wAmp[2];
    // wAmp[0] = 32767; // 5V TTL
    wAmp[0] = 32767; // 5V TTL
    wAmp[1] = 0; // 0 V baseline


    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 4; // continuous reward
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 1; // yet another way to set the number of repeats of the waveform

    //res = cbSdkSetComment(0, 0, 0, "Water reward");
    res = cbSdkSetAnalogOutput(0, currentWaterAOUT, &wf, NULL);
#else
    qDebug() << "Start flush";
#endif
}

void SpkAndBehav::stop_water(){
#if DEMO == 0
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[1];
    wDur[0] = 300; // 10 ms at 30 000 Hz. Constantly ON

    /// voltage of the phases of the waveform
    int16_t wAmp[2];
    wAmp[0] = 0; // 5V TTL

    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 10; // continuous reward
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 1; // yet another way to set the number of repeats of the waveform

    //res = cbSdkSetComment(0, 0, 0, "Water reward");
    res = cbSdkSetAnalogOutput(0, currentWaterAOUT, &wf, NULL);
#else
    qDebug() << "Stop water";
#endif
}

void SpkAndBehav::start_cameraTrig(){
#if DEMO == 0
    /// Set a waveform that will be played every time there is a reward
    /// duration of the phases of the waveform
    uint16_t wDur[2];
    wDur[0] = 150; // 5 ms up at 30 000 Hz
    wDur[1] = 150; // 5 ms down at 30 000 Hz

    /// voltage of the phases of the waveform
    int16_t wAmp[2];  // -32767 for -5V and +32767 for 5V
    wAmp[0] = 32767; // 5V TTL
    wAmp[1] = 0; // 0V

    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = SessionPRD*60*100; // depends on session duration at 100Hz
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 2; // yet another way to set the number of repeats of the waveform

    res = cbSdkSetAnalogOutput(0, currentCameraAOUT, &wf, NULL);
#else
    qDebug() << "Start Camera trigger";
#endif
}

void SpkAndBehav::stop_cameraTrig(){
#if DEMO == 0
    ///UINT16 channel = 145; //Analog output channel number on NSP (channel 1)
    uint16_t wDur[2];
    wDur[0] = 150; // 5 ms up at 30 000 Hz
    wDur[1] = 150; // 5 ms down at 30 000 Hz

    /// voltage of the phases of the waveform
    int16_t wAmp[2];  // -32767 for -5V and +32767 for 5V
    wAmp[0] = 0; // 5V TTL
    wAmp[1] = 0; // 0V

    /// Define the analog waveform
    cbSdkWaveformData wf;
    wf.type = cbSdkWaveform_PARAMETERS; // through the shape defined in the repeats parameter
    wf.repeats = 0; // continuous pulsing
    wf.trig = cbSdkWaveformTrigger_NONE; // Instant software trigger
    wf.trigChan = 0; // Channel trigger: not used here
    wf.trigValue = 0; // Channel trigger: not used here
    wf.trigNum = 0; // Channel trigger: not used here
    wf.offset = 0; // Channel trigger: not used here
    wf.duration = wDur; // duration of the phases of the waveform
    wf.amplitude = wAmp;// voltage over the phases of the waveform
    wf.phases = 2; // yet another way to set the number of repeats of the waveform

    res = cbSdkSetAnalogOutput(0, currentCameraAOUT, &wf, NULL);
#else
    qDebug() << "Stop Camera trigger";
#endif
}

void SpkAndBehav::stopSession()
{
#if DEMO == 0
    /// init the recording
    res = cbSdkSetComment(0, 0, 0, "Stop session");

    /// close the recording file
    res = cbSdkSetFileConfig(0, currentSessionFileName.toStdString().c_str(), "", false, cbFILECFG_OPT_NONE);

    /// if the session was a playback, close the file
    //inputFile->close();  // Application is crashing on pressing the stop session button due to this line.

    /// stop projector
    AlpDevHalt( nDevId );
    AlpDevFree( nDevId );

    /// reset lick and reward variables
    LickHist = 0;
    rewardInterval = 0;

#endif
    param = Protocole();
}

void SpkAndBehav::initControlSession(){

    qDebug()<< "control session";
    StartTime_s = QDateTime::currentDateTime().toTime_t();

    /// define the 30k continuous acquisition, no filter, of the data across all 32 channels
    cbPKT_CHANINFO chan_info;

    for (int i = 0; i < 32 ; i++){
        res = cbSdkGetChannelConfig(0, 1, &chan_info); //get current channel configuration
        //change configuration
        chan_info.smpfilter = 0; //continuous filter (none)
        chan_info.smpgroup = 5; //continuous sampling rate (30kHz)
        //set channel configuration
        res = cbSdkSetChannelConfig(0, i+1, &chan_info); //note: channels start at 1
    }

    /// Configure the trial to include both continuous and event data
    uint16_t bActive = 1; // reset the trial
    uint16_t uBegChan   = 1;
    uint32_t uBegMask   = 0;
    uint32_t uBegVal    = 0;
    uint16_t uEndChan   = 32;
    uint32_t uEndMask   = 0;
    uint32_t uEndVal    = 0;
    bool   bDouble    = false;
    bool   bAbsolute  = false;
    uint32_t uWaveforms = 0;
    uint32_t uConts     = cbSdk_CONTINUOUS_DATA_SAMPLES; // record the continuous data
    uint32_t uEvents    = cbSdk_EVENT_DATA_SAMPLES;
    uint32_t uComments  = 0; // should we record the comments ?
    uint32_t uTrackings = 0;

    res = cbSdkSetTrialConfig(0, bActive, uBegChan, uBegMask, uBegVal, uEndChan, uEndMask, uEndVal, bDouble, uWaveforms, uConts, uEvents, uComments, uTrackings, bAbsolute);

    /// Init the data recording: look at the next session number in the data folder of the current mouse
    /// create folder that will contain the data
    paramMut.lock();
    QString currentSessionFolderName = currentDataFolder + "mouse" + QString::number(Mouse.load()) + "/session" + QString::number(Session.load()) + '/';
    paramMut.unlock();
    /// create folder that will contain the data
    QDir dir;
    dir.mkdir(currentSessionFolderName);
    QString FileName = currentSessionFolderName + "data";
    /// Init the file.
    res = cbSdkSetFileConfig(0, FileName.toStdString().c_str(), "", true, cbFILECFG_OPT_NONE);

    /// Sleep for few seconds to allow launch of the recording
    Sleep(2000);

    /// init the recording
    res = cbSdkSetComment(0, 0, 0, "Init session");

    /// Log as events the mouse, session
    res = cbSdkSetComment(0, 0, 0, ("Mouse:"+ QString::number(Mouse.load())).toLocal8Bit().constData());
    res = cbSdkSetComment(0, 0, 0, ("Session:"+ QString::number(Session.load())).toLocal8Bit().constData());
    /// Log as events the date
    QDateTime time = QDateTime::currentDateTime();
    res = cbSdkSetComment(0, 0, 0, ("Date:"+ time.toString("dd/MM/yy")).toStdString().c_str()); // the date
    res = cbSdkSetComment(0, 0, 0, ("Hour:"+ time.toString("hh:mm:ss")).toStdString().c_str()); // the hour/minutes/seconds.


    /// Update the parameters that are used
    paramMut.lock();
    currentParam = param;
    paramMut.unlock();

    /// Load the Parameters
    rewardStrength = currentParam.rewardStrength;
    RewardLowerThd.store(currentParam.rewardLower);
    RewardUpperThd.store(currentParam.rewardUpper);
    timeOut = currentParam.timeOut;
    rewardInterval = currentParam.rewardInterval;
    rewardThd = currentParam.rewardThd;
    input = currentParam.input;
    brightness = currentParam.brightness;
    frameDuration = currentParam.frameDuration;
    frameIllumination = currentParam.frameIllumination;
    FrameList = currentParam.FrameList;

    /// Log as events some parameters
    cbSdkSetComment(0, 0, 0, ("brightness:" + QString::number(brightness)).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("frameDuration:" + QString::number(frameDuration)).toStdString().c_str());
    cbSdkSetComment(0, 0, 0, ("frameIllumination:" + QString::number(frameIllumination)).toStdString().c_str());

    /// init the reading of the file
    inputFile = new QFile(currentDataFolder + "Protocoles/" + input + ".txt");
    if(inputFile->open(QIODevice::ReadOnly)){
        inputStream= new QTextStream(inputFile);
        BufferList.clear();
        QString line;
        do {
            line = inputStream->readLine();
            BufferList.append(line.toInt(NULL,10));
        } while(!line.isNull());
        BufferList.removeLast(); // remove empty last line
        inputFile->close();
        qDebug() << "Reading finished";
    }

    /// start the projector
    const int seq_count = FrameList.count();
    const int frame_count = FrameList[0].count();
    nSeqId_list = QVector<ALP_ID>(seq_count);
    /// Allocate the ALP high-speed device
    if (ALP_OK != AlpDevAlloc( ALP_DEFAULT, ALP_DEFAULT, &nDevId )){
        qDebug() << "AlpDevAlloc failed. Check projector";
        return;
    }
    /// queue mode...
    if (ALP_OK != AlpProjControl(nDevId, ALP_PROJ_QUEUE_MODE, ALP_PROJ_SEQUENCE_QUEUE)){
        qDebug()<< "AlpProjControl failed. Check projector";
        return;
    }

    /// Allocate the sequences in FrameList (nPatterns parameter)
    for (int i = 0; i < seq_count ; i++) {
        /// init a sequence
        if (ALP_OK != AlpSeqAlloc( nDevId, 1, frame_count, &(nSeqId_list[i]))){
            qDebug() << "AlpSeqAlloc failed. Check projector";
            return;
        }
        /// Fill the buffer with the images
        for (int j = 0; j < frame_count; j++)
        {
            if(ALP_OK != AlpSeqPut( nDevId, nSeqId_list[i], j, 1, FrameList[i][j].bits())){
                qDebug() << "AlpSeqPut failed. Check projector";
                return;
            }
        }
        if (ALP_OK != AlpSeqTiming(nDevId, nSeqId_list[i],frameIllumination, frameDuration, ALP_DEFAULT, ALP_DEFAULT, ALP_DEFAULT)){
            qDebug()<< "AlpSeqTiming failed. Check projector";
            return;
        }
        if (ALP_OK != AlpSeqControl(nDevId, nSeqId_list[i],ALP_BITNUM,1)){
            qDebug() << "AlpSeqControl failed. Check projector";
            return;
        }
    }

    /// initialize LED
    if (ALP_OK != AlpLedAlloc( nDevId, ALP_HLD_PT120TE_BLUE, NULL, &AlpLedId )){
        qDebug() << "AlpLedAlloc failed. Check projector";
        return;
    }
    if (ALP_OK != AlpLedControl( nDevId, AlpLedId, ALP_LED_BRIGHTNESS, brightness))
    {
        qDebug() << "AlpLedControl failed. Check projector";
        return;
    }

    if (ALP_OK != AlpLedControl( nDevId, AlpLedId, ALP_LED_SET_CURRENT, 30000))
    {
        qDebug() << "AlpLedControl failed. Check projector";
        return;
    }

    /// Main LOOP
    //    if (ALP_OK != AlpProjStartCont( nDevId, nSeqId_list[0])){
    //        qDebug() << "AlpProjStartCont failed. Check projector"; // init display, ideally a black frame
    //        return;
    //    }
}
