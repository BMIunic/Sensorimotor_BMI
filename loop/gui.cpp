//////////////////////////////////////////////////////////////////////////////
/// Sensorimotor BMI GUI (2016)
/// This file containes the definations of all the widgets on the GUI

#include "gui.h"

Gui::Gui(QWidget *parent) :
    QWidget(parent)
{
    /// LOAD THE PARAMETER FILE
    QString path = QApplication::applicationDirPath();

    /// From program ini file, get the data dir, the data
    QString IniFile = path + "/Parameters.ini";

    /// check if main parameter file exists and if yes: Is it really a file and no directory?
    QFileInfo check_inifile(IniFile);
    if (check_inifile.exists() && check_inifile.isFile()) {
        QSettings settings(IniFile, QSettings::IniFormat);
        dataFolder = settings.value("dataFolder").toString();
        cameraAOUT = settings.value("cameraAOUT").toInt();
        waterAOUT = settings.value("waterAOUT").toInt();
        lickDIN = settings.value("lickDIN").toInt();
        maxSpkCount = settings.value("bufferSPIKECOUNT").toInt();
        binsizeFR = settings.value("binsizeFIRINGRATE(ms)").toInt();
        WaitingPRD = settings.value("waitingPeriod(min)").toInt();
        RoutineTime = settings.value("routineCall(ms)").toInt();
        SessionPRD = settings.value("sessionPeriod(min)").toInt();
        RandProtocolID = settings.value("protocolID").toInt();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("The main parameter file cannot be found.");
        msgBox.exec();
        QApplication::quit();
    }

    QFileInfo check_datafolder(dataFolder);
    /// check if directory exists?
    if (!check_datafolder.exists()) {
        QMessageBox msgBox;
        msgBox.setText("Data directory does not exist.");
        msgBox.exec();
        QApplication::quit();
    }

    param = Protocole();
    currentParam = param;
    currentDataFolder = dataFolder;

    /// INIT SPIKE THREAD
    Active.store(1); // Spiking thread active, not yet projection thread

    /// Init reward counter
    RewardCounter.store(0);

    /// Init the thread that reads the spikes
    spkandbehav_thread = new QThread();
    spkandbehav = new SpkAndBehav();
    spkandbehav->moveToThread(spkandbehav_thread);
    spkandbehav_thread->start();

    /// INIT GUI;
    /// select mouse.
    m_mouseComboBox = new QComboBox(this);
    ListMice();
    for (int i = 0; i< mouseIDlist.count(); i++){
        m_mouseComboBox->addItem(QString::number(mouseIDlist[i]), mouseIDlist[i]);
    }
    m_mouseComboBox->setCurrentIndex(-1); // start with empty combobox

    /// select protocole.
    m_protComboBox = new QComboBox(this);
    ListProtocoles();
    for (int i = 0; i< ProtocolesList.count(); i++){
        QStringList list1 = ProtocolesList[i].split(".");
        m_protComboBox->addItem(list1[0], ProtocolesList[i]);
    }
    m_protComboBox->setCurrentIndex(-1); // start with empty combobox

    /// activate high speed camera.
    m_cameraCheckBox = new QCheckBox("Camera", this); // check for control experiments

    /// control session
    m_controlsCheckBox = new QCheckBox("Control Exp", this);

    /// Start/ session.
    sessionButton = new QPushButton("Start session");
    sessionButton->setCheckable(true);
    connect(sessionButton, SIGNAL(toggled(bool)), this, SLOT(sessionON_OFF(bool)));

    /// Generate protocol id
    protocolIdButton = new QPushButton("Protocol id "+QString::number(RandProtocolID.load()));
    connect(protocolIdButton, SIGNAL(clicked()), this, SLOT(protocolIdButton_clicked()));

    /// Select electrode channel
    m_channelListWidget = new QListWidget(this);
    m_channelListCheckbox = new QCheckBox(this);

    /// Select unit
    m_unitListWidget = new QListWidget(this);

    connect(m_mouseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMouse()));
    connect(m_channelListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(updateChannel(QModelIndex)));
    connect(m_unitListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(updateUnit(QModelIndex)));
    connect(m_channelListCheckbox, SIGNAL(toggled(bool)), this, SLOT(allChannels(bool)));

    /// Create label that will be displaying the projected images
    imageLabel = new QLabel();
    imageLabel->setScaledContents(true);
    imageLabel->setMaximumSize(500,400);

    baseline_image = new QImage(1024, 768, QImage::Format_Grayscale8);
    baseline_image->fill(128);
    imageLabel->setPixmap(QPixmap::fromImage(*baseline_image));

    /// Create chart view with qt charts
    chart = new Chart();
    chart->legend()->hide();
    m_chartView = new QChartView(chart);
    m_chartView -> setRenderHint(QPainter::Antialiasing);

    /// Select min and max firing rate
    m_blwSpinBox = new QDoubleSpinBox(this);
    m_blwSpinBox->setValue(0);
    m_abvSpinBox = new QDoubleSpinBox(this);
    m_abvSpinBox->setValue(1);
    m_abvSpinBox->setMaximum(10000);

    /// connect to the firing rate display
    UpperThd.store(1);
    LowerThd.store(0);
    connect(m_blwSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLowerThd(double)));
    connect(m_abvSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateUpperThd(double)));

    /// Select min and max firing rate
    m_blwSDSpinBox = new QDoubleSpinBox(this);
    m_blwSDSpinBox->setValue(1);
    m_abvSDSpinBox = new QDoubleSpinBox(this);
    m_abvSDSpinBox->setValue(1);

    /// connect to the firing rate display for SD based thresholds
    connect(m_blwSDSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLowerSDThd(double)));
    connect(m_abvSDSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateUpperSDThd(double)));

    /// qt label for displaying the mean and sd of firing rate after the waiting period ends
    label_mean = new QLabel(this);
    label_std = new QLabel(this);

    /// Push button to calculate the mean and sd of neuron
    meanrateCALCButton = new QPushButton("Start waiting");
    meanrateCALCButton->setCheckable(true);
    connect(meanrateCALCButton, SIGNAL(toggled(bool)), this, SLOT(calc_meanFR(bool)));

    /// Select min and max reward bounds
    m_blwRewardSpinBox = new QSpinBox(this);
    m_blwRewardSpinBox->setValue(1);
    m_abvRewardSpinBox = new QSpinBox(this);
    m_abvRewardSpinBox->setValue(1);

    /// connect reward bounds to spinbox to slots to update the firing rate display
    connect(m_blwRewardSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRewardLowerThd(int)));
    connect(m_abvRewardSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRewardUpperThd(int)));

    /// qt lable to display the number of rewards animal got in one session
    label_reward = new QLabel(this);

    /// Select min and max firing rate display
    m_blwDispSpinBox = new QDoubleSpinBox(this);
    m_blwDispSpinBox->setValue(-2);
    m_abvDispSpinBox = new QDoubleSpinBox(this);
    m_abvDispSpinBox->setValue(10);
    m_abvDispSpinBox->setMaximum(10000);

    connect(m_blwDispSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateLowerDisplay(double)));
    connect(m_abvDispSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateUpperDisplay(double)));

    /// Continous rewarding ON/OFF
    rewardONButton = new QPushButton("Start continous reward");
    rewardONButton->setCheckable(true);
    connect(rewardONButton, SIGNAL(toggled(bool)), this, SLOT(rewardON_OFF(bool)));

    /// Flush ON/OFF
    flushONButton = new QPushButton("Flush");
    flushONButton->setCheckable(true);
    connect(flushONButton, SIGNAL(toggled(bool)), this, SLOT(flushON_OFF(bool)));

    /// One Free reward ON
    oneRewardONButton = new QPushButton("One Free Reward");
    connect(oneRewardONButton, SIGNAL(clicked()), this, SLOT(oneFreeRewardON()));

    /// Defination of channels widget
    m_channelListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_channelListWidget->setMaximumSize(40,1000);
    new QListWidgetItem(tr("1"), m_channelListWidget);
    new QListWidgetItem(tr("2"), m_channelListWidget);
    new QListWidgetItem(tr("3"), m_channelListWidget);
    new QListWidgetItem(tr("4"), m_channelListWidget);
    new QListWidgetItem(tr("5"), m_channelListWidget);
    new QListWidgetItem(tr("6"), m_channelListWidget);
    new QListWidgetItem(tr("7"), m_channelListWidget);
    new QListWidgetItem(tr("8"), m_channelListWidget);
    new QListWidgetItem(tr("9"), m_channelListWidget);
    new QListWidgetItem(tr("10"), m_channelListWidget);
    new QListWidgetItem(tr("11"), m_channelListWidget);
    new QListWidgetItem(tr("12"), m_channelListWidget);
    new QListWidgetItem(tr("13"), m_channelListWidget);
    new QListWidgetItem(tr("14"), m_channelListWidget);
    new QListWidgetItem(tr("15"), m_channelListWidget);
    new QListWidgetItem(tr("16"), m_channelListWidget);
    new QListWidgetItem(tr("17"), m_channelListWidget);
    new QListWidgetItem(tr("18"), m_channelListWidget);
    new QListWidgetItem(tr("19"), m_channelListWidget);
    new QListWidgetItem(tr("20"), m_channelListWidget);
    new QListWidgetItem(tr("21"), m_channelListWidget);
    new QListWidgetItem(tr("22"), m_channelListWidget);
    new QListWidgetItem(tr("23"), m_channelListWidget);
    new QListWidgetItem(tr("24"), m_channelListWidget);
    new QListWidgetItem(tr("25"), m_channelListWidget);
    new QListWidgetItem(tr("26"), m_channelListWidget);
    new QListWidgetItem(tr("27"), m_channelListWidget);
    new QListWidgetItem(tr("28"), m_channelListWidget);
    new QListWidgetItem(tr("29"), m_channelListWidget);
    new QListWidgetItem(tr("30"), m_channelListWidget);
    new QListWidgetItem(tr("31"), m_channelListWidget);
    new QListWidgetItem(tr("32"), m_channelListWidget);

    /// Defination of units widget
    m_unitListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_unitListWidget->setMaximumSize(40,140);
    new QListWidgetItem(tr("0"), m_unitListWidget);
    new QListWidgetItem(tr("1"), m_unitListWidget);
    new QListWidgetItem(tr("2"), m_unitListWidget);
    new QListWidgetItem(tr("3"), m_unitListWidget);
    new QListWidgetItem(tr("4"), m_unitListWidget);
    new QListWidgetItem(tr("5"), m_unitListWidget);

    /// Create layout of window
    /// Layout to select mouse and protocol
    ComboLayout = new QFormLayout();
    ComboLayout->addRow("Mouse", m_mouseComboBox);
    ComboLayout->addRow("Protocole", m_protComboBox);

    /// Layout to select channels and units
    ListWidgetLayout = new QFormLayout();
    ListWidgetLayout->addRow("All Channels", m_channelListCheckbox);
    ListWidgetLayout->addRow("Channels", m_channelListWidget);
    ListWidgetLayout->addRow("Units", m_unitListWidget);

    /// Layout to arrange firing rate and reward  threhold parameters
    SpinboxLayout2 = new QFormLayout();
    SpinboxLayout2->addRow("Max firing", m_abvSpinBox);
    SpinboxLayout2->addRow("Min firing", m_blwSpinBox);
    SpinboxLayout2->addRow("Max firing Std", m_abvSDSpinBox);
    SpinboxLayout2->addRow("Min firing Std", m_blwSDSpinBox);
    SpinboxLayout2->addRow("Mean FR", label_mean);
    SpinboxLayout2->addRow("Std FR", label_std);
    SpinboxLayout2->addRow("Upper reward", m_abvRewardSpinBox);
    SpinboxLayout2->addRow("Lower reward", m_blwRewardSpinBox);
    SpinboxLayout2->addRow("Reward counter",label_reward);
    ControlLayout2 = new QHBoxLayout();
    ControlLayout2->addLayout(ListWidgetLayout);
    ControlLayout2->addLayout(SpinboxLayout2);

    /// 2 - stack spinboxes with start and display buttons
    ControlLayout = new QVBoxLayout();
    ControlLayout->addLayout(ComboLayout);
    ControlLayout3 = new QHBoxLayout();
    ControlLayout3->addWidget(m_cameraCheckBox);
    ControlLayout3->addWidget(m_controlsCheckBox);
    ControlLayout3->addWidget(protocolIdButton);
    ControlLayout3->addWidget(meanrateCALCButton);
    ControlLayout3->addWidget(sessionButton);
    ControlLayout->addLayout(ControlLayout3);

    /// QFrame to dispaly the projected images
    line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    ControlLayout->addWidget(line);

    ControlLayout->addLayout(ControlLayout2);

    WaterLayout = new QHBoxLayout();
    WaterLayout->addWidget(rewardONButton);
    WaterLayout->addWidget(flushONButton);
    WaterLayout->addWidget(oneRewardONButton);
    ControlLayout->addLayout(WaterLayout);

    line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    ControlLayout->addWidget(line2);

    groupBox = new QGroupBox("Settings");
    groupBox->setLayout(ControlLayout);

    /// 3 - Lateral design with label that will show the current display
    SecondLevel = new QHBoxLayout();
    SecondLevel->addWidget(groupBox);
    SecondLevel->addStretch();
    SecondLevel->addWidget(imageLabel);
    SecondLevel->addStretch();

    SpinboxLayout3 = new QFormLayout();
    SpinboxLayout3->addRow("top Hz", m_abvDispSpinBox);
    SpinboxLayout3->addRow("bottom Hz", m_blwDispSpinBox);

    ControlLayout4 = new QHBoxLayout();
    ControlLayout4->addLayout(SpinboxLayout3);
    ControlLayout4->addWidget(m_chartView);

    FirstLevel = new QVBoxLayout();
    FirstLevel->addLayout(SecondLevel);
    FirstLevel->addLayout(ControlLayout4);
    setLayout(FirstLevel);

    /// init the timer that updates the graphs
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(update_displays()));
    m_timer.setInterval(100); // ms
    m_timer.start();

    /// flag declerations
    meanFR_active_before = 0;
    session_active_before = 0;
}

Gui::~Gui()
{
    //  both threads
    Active.store(0);
}

/// Slots for manually changing of the thresholds
void Gui::updateLowerThd(double lower){

    // calculate the threshold value and update the GUI
    LowerThd.store((unsigned int)lower);
    chart->updateBlwCursors((float)lower);

    // compute the position of the upper and lower reward limits in terms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("TL:" + QString::number(LowerThd.load())).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

void Gui::updateUpperThd(double upper){

    // calculate the threshold value and update the GUI
    UpperThd.store((unsigned int)upper);
    chart->updateAbvCursors((float)upper);

    // compute the position of the upper and lower reward limits in tearms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("TU:" + QString::number(UpperThd.load())).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

/// Slots for changing threshold based on mean +/- standard deviation (SD)
void Gui::updateLowerSDThd(double lowerSD){

    // calculate the threshold value and update the GUI
    LowerThd.store(MeanFR - (float)lowerSD*SdFR);
    chart->updateBlwCursors(MeanFR - (float)lowerSD*SdFR);
    m_blwSpinBox->setValue(LowerThd.load());

    // compute the position of the upper and lower reward limits in terms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("TL:" + QString::number(LowerThd.load())).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

/// Slots for changing threshold based on mean +/- standard deviation (SD)
void Gui::updateUpperSDThd(double upperSD){

    // calculate the threshold value and update the GUI
    UpperThd.store(MeanFR + (float)upperSD*SdFR);
    chart->updateAbvCursors(MeanFR + (float)upperSD*SdFR);
    m_abvSpinBox->setValue(UpperThd.load());

    // compute the position of the upper and lower reward limits in tearms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("TU:" + QString::number(UpperThd.load())).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

/// Slots for changing the reward width by changing the upper reward and lower reward positions
void Gui::updateRewardLowerThd(int lowerRwd){

    RewardLowerThd.store(lowerRwd);
    // compute the position of the upper and lower reward limits in tearms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("rwdFrameTL:" + QString::number(lowerRwd)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

/// Slots for changing the reward width by changing the upper reward and lower reward positions
void Gui::updateRewardUpperThd(int upperRwd){

    RewardUpperThd.store(upperRwd);
    // compute the position of the upper and lower reward limits in tearms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    // Log as events
    if (Active.load() > 1){
        cbSdkSetComment(0, 0, 0, ("rwdFrameTU:" + QString::number(upperRwd)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTL:" + QString::number(rewardLowerBfr)).toStdString().c_str());
        cbSdkSetComment(0, 0, 0, ("rwdTU:" + QString::number(rewardUpperBfr)).toStdString().c_str());
    }
}

/// slot for calaculating the mean and sd of neuronal activity.
void Gui::calc_meanFR(bool meanCALC_ON){
    if (meanCALC_ON){
        GetLastSession();
        Session.store(Session.load()+1);
        meanFR_active_before = 1;
        initWaiting();
        meanrateCALCButton->setText("Calculating Mean");
        MeanFR_active.store(1);
        MeanFR_StartTime = (float)(std::clock()/CLOCKS_PER_SEC);
    }
    else {
        meanrateCALCButton->setText("Start waiting");
        MeanFR_active.store(0);
    }
    LoadProtocole(); // load current protocole
}

/// slot for initialize the recording of waiting period and session after "Start wating" button is pressed
void Gui::initWaiting(){
    StartTime_s = QDateTime::currentDateTime().toTime_t();

    /// define the 30k continuous acquisition, no filter, of the data across all 32 channels
    cbPKT_CHANINFO chan_info;

    for (int i = 0; i < 32 ; i++){
        //get current channel configuration
        res = cbSdkGetChannelConfig(0, i+1, &chan_info);

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
    uint32_t uComments  = 1000; // should we record the comments ?
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

    /// Sleep for few seconds to allow the launch of recording dialogbox
    Sleep(1000);

    /// initiate the recording
    res = cbSdkSetComment(0, 0, 0, "Init waiting");
}

void Gui::updateUpperDisplay(double upper){
    chart->updateAbvDisplay(upper);
}

void Gui::updateLowerDisplay(double lower){
    chart->updateBlwDisplay(lower);
}

/// When the gui enters active state
void Gui::LoadProtocole()
{

    paramMut.lock();
    param = Protocole(m_protComboBox->currentData().toString(), Mouse.load());
    currentParam = param;
    paramMut.unlock();

    // compute the position of the upper and lower reward limits in terms of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }

    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    m_blwRewardSpinBox->setValue((int)currentParam.rewardLower);
    m_abvRewardSpinBox->setValue((int)currentParam.rewardUpper);

    label_reward->setText(QString::number(RewardCounter.load()));
}

/// In resting state
void Gui::LoadNullProtocole()
{
    paramMut.lock();
    param = Protocole();
    currentParam = param;
    paramMut.unlock();

    // compute the position of the upper and lower reward limits in tersm of firing rate
    double rewardLowerBfr;
    double rewardUpperBfr;
    if (currentParam.FrameList.count() > 0)
    {
        rewardLowerBfr = (RewardLowerThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
        rewardUpperBfr = (RewardUpperThd.load()-0.5)/((double)(currentParam.FrameList.count()-1))*(UpperThd.load() - LowerThd.load())+LowerThd.load();
    } else {
        rewardLowerBfr = 0;
        rewardUpperBfr = 0;
    }
    chart->updateBlwReward(rewardLowerBfr);
    chart->updateAbvReward(rewardUpperBfr);

    m_blwRewardSpinBox->setValue(RewardLowerThd.load());
    m_abvRewardSpinBox->setValue(RewardUpperThd.load());
}

/// Entry point to toggle session ON/OFF state
void Gui::sessionON_OFF(bool session_ON){
    if(session_ON){
        startSession();
    } else {
        stopSession();
    }
}

/// Returns a random number corresponding to a protocol id
void Gui::protocolIdButton_clicked(){

    // draw protocol id randomly from numbers 0-4
    RandProtocolID.store(qrand()%4); // generate random numbers from 0-4

    // update the protocol id on GUI
    protocolIdButton->setText("Protocol id "+QString::number(RandProtocolID.load()));

    // update the parameters.ini file
    QString path = QApplication::applicationDirPath();
    QString IniFile = path + "/Parameters.ini";
    QFile settingFile(IniFile);
    if(settingFile.open(QFile::WriteOnly | QFile::Text)){
        QTextStream out(&settingFile);
        out << "dataFolder = "+ dataFolder << endl;
        out << "cameraAOUT = "+ QString::number(cameraAOUT) << endl;
        out << "waterAOUT = "+ QString::number(waterAOUT) << endl;
        out << "lickDIN = "+ QString::number(lickDIN) << endl;
        out << "bufferSPIKECOUNT = "+ QString::number(maxSpkCount) << endl;
        out << "binsizeFIRINGRATE(ms) = "+ QString::number(binsizeFR) << endl;
        out << "waitingPeriod(min) = "+ QString::number(WaitingPRD.load()) << endl;
        out << "routineCall(ms) = "+ QString::number(RoutineTime.load()) << endl;
        out << "sessionPeriod(min) = "+ QString::number(SessionPRD.load()) << endl;
        out << "protocolID = "+ QString::number(RandProtocolID.load()) << endl;
    }

    settingFile.flush();
    settingFile.close();
}

/// Monitor all channels (default)
void Gui::allChannels(bool allChannels_ON){
    if(allChannels_ON){
        res = cbSdkSetComment(0, 0, 0, "Chan=0");
        m_channelListWidget->setEnabled(false);
        m_channelListWidget->selectAll();
        QList<QListWidgetItem *> ch = m_channelListWidget->selectedItems();
        for (int i=0; i<ch.count(); i++)
        {
            m_channelListWidget->selectedItems()[i]->setBackground(Qt::yellow);
            Channel.append(m_channelListWidget->selectedItems()[i]->text().toInt());
        }
    } else {
        m_channelListWidget->setEnabled(true);
        m_channelListWidget->clearSelection();
        Channel.clear();
        for (int i=0; i<m_channelListWidget->count(); i++)
        {
            QListWidgetItem* item = m_channelListWidget->item(i);
            item->setBackground(Qt::white);
        }
    }
}

/// Get selected channels
void Gui::updateChannel(const QModelIndex &){
    Channel.clear();

    for (int i=0; i<m_channelListWidget->count(); i++)
    {
        QListWidgetItem* item = m_channelListWidget->item(i);
        item->setBackground(Qt::white);
    }

    QList<QListWidgetItem *> ch = m_channelListWidget->selectedItems();
    for (int i=0; i<ch.count(); i++)
    {
        m_channelListWidget->selectedItems()[i]->setBackground(Qt::yellow);
        Channel.append(m_channelListWidget->selectedItems()[i]->text().toInt());
    }
    //updateChannelUnitComment(Unit,Channel);
}

/// Get selected units
void Gui::updateUnit(const QModelIndex &){
    Unit.clear();

    for (int i=0; i<m_unitListWidget->count(); i++)
    {
        QListWidgetItem* item = m_unitListWidget->item(i);
        item->setBackground(Qt::white);
    }

    QList<QListWidgetItem *> Un =  m_unitListWidget->selectedItems();
    for (int i=0; i<Un.count(); i++)
    {
        m_unitListWidget->selectedItems()[i]->setBackgroundColor(Qt::yellow);
        Unit.append(m_unitListWidget->selectedItems()[i]->text().toInt());
    }
    updateChannelUnitComment(Unit,Channel);
}

/// Set selected channels and unit to stream data from the NSP plugin
void Gui::updateChannelUnitComment(QVector<int> Unit,QVector<int> Channel){
    /// Read all the channels and units as a Qstring for logging as comments
    cbSdkSetComment(0,0,0,("Unit=" + QString::number(Unit.at(0))).toStdString().c_str());
    cbSdkSetComment(0,0,0,("Channel=" + QString::number(Channel.at(0))).toStdString().c_str());
}

void Gui::setMouse(){
    Mouse.store((unsigned int)(m_mouseComboBox->currentData().toInt()));
    GetLastSession();
    sessionButton->setText("Start session "+QString::number(Session.load()+1));
}

void Gui::rewardON_OFF(bool reward_ON){
    if(reward_ON){
        flushONButton->setChecked(false);
        flushONButton->setText("Start Flush");
        rewardONButton->setText("Stop continous reward");
        Water.store(1);
    } else {
        rewardONButton->setText("Start continous reward");
        Water.store(0);
    }
}

void Gui::flushON_OFF(bool flush_ON){
    if(flush_ON){
        // release the free reward button
        rewardONButton->setChecked(false);
        rewardONButton->setText("Start continous reward");
        flushONButton->setText("Stop Flush");
        Water.store(2); // set the global variable
    } else {
        flushONButton->setText("Start Flush");
        Water.store(0);
    }
}

void Gui::oneFreeRewardON(){

    Water.store(3); // set the global variable
}

/// Update GUI with the frequency of a preset Qt Timer
void Gui::update_displays(){

    // update the graph
    chart->update();
    // update image

    if (currentParam.GuiFrameList.count() == 0)
    {
        imageLabel->setPixmap(QPixmap::fromImage(*baseline_image));
    }
    else
    {
        imageLabel->setPixmap(QPixmap::fromImage(currentParam.GuiFrameList[FrameIdx.load()]));
    }

    // Update calculate mean status button
    if (MeanFR_active.load()==0 &&  meanFR_active_before==1){
        meanrateCALCButton->setChecked(false);
        meanrateCALCButton->setText("Start waiting");
        meanrateCALCButton->setEnabled(false);
        sessionButton->setEnabled(true);
        label_mean->setText(QString::number(MeanFR));
        label_std->setText(QString::number(SdFR));
        meanFR_active_before = 0;
    }

    // Update stop session status button if session period is over
    if (Active.load() == 1 && session_active_before > 1){
        stopSession();
        session_active_before = 0;
    }

    // Update the reward counter label to display the number of rewards on the GUI
    if (RewardCounter.load() != 0){
        label_reward->setText(QString::number(RewardCounter.load()));
    }

}

/// Implementation of start session called by the entry point slot
void Gui::startSession(){
    // Init the thread that projects the images
    // data is in the thread

    // Store the time of session start in a global variable
    Session_StartTime = (float)(std::clock()/CLOCKS_PER_SEC);

    session_active_before = 2;

    if (!(m_controlsCheckBox->isChecked())){
        if (m_cameraCheckBox->isChecked())
        {
            Active.store(3); // turn on session and start sending camera frame triggers
        } else {
            Active.store(2); // session without camera
            qDebug() << "S2";
        }
    }

    if (m_controlsCheckBox->isChecked()){

        Active.store(4); // session for control experiments
        GetLastSession();
        Session.store(Session.load()+1);
        LoadProtocole();
    }
    //LoadProtocole(); // load current protocole

    sessionButton->setText("Stop session "+QString::number(Session.load()));
}

/// Implementation of stop session called by the entry point slot
void Gui::stopSession(){

    sessionButton->setText("Start session "+QString::number(Session.load()+1));
    sessionButton->setChecked(false);
    //sessionButton->setEnabled(false);
    meanrateCALCButton->setEnabled(true);
    // turn off the session, not the whole GUI
    Active.store(1);
    // replace the current parameters with the 'no trials' params
    LoadNullProtocole();
}

/// Get the folder list from a directory
void Gui::ListMice(){
    // List the mouse IDs in the data folder
    paramMut.lock();
    QDir root = QDir(dataFolder); // initialize the directory that will be parsed
    paramMut.unlock();

    QStringList mouseNameList = root.entryList();
    QList<int> mouseIDs = QList<int>();
    // identify the mouse IDs
    for (int i = 0; i < mouseNameList.count(); i++){
        if ((mouseNameList[i].count() > 5) && (QString::compare(mouseNameList[i].left(5), QString("mouse"), Qt::CaseSensitive) == 0))
        {
            int numLength = mouseNameList[i].count() - 5; // extract the part of the string that contains the number
            QString IDstr = mouseNameList[i].right(numLength);
            mouseIDs.append(IDstr.toInt());
        }
    }
    qSort(mouseIDs.begin(), mouseIDs.end());
    mouseIDlist = mouseIDs;
}

/// Get the last session number from a directory
void Gui::GetLastSession(){
    // List the session numbers in the current mouse folder
    paramMut.lock();
    QDir root = QDir(dataFolder + "/mouse" + QString::number(Mouse.load()));
    paramMut.unlock();
    QStringList sessionNameList = root.entryList();
    QList<unsigned int> sessionIDs = QList<unsigned int>();
    // identify the session IDs
    for (int i = 0; i < sessionNameList.count(); i++){
        QString IDstr;
        if (QString::compare(sessionNameList[i].left(7), "session") == 0)
        {
            int numLength = sessionNameList[i].count() - 7; // extract the part of the string that contains the number
            IDstr = sessionNameList[i].right(numLength);
        }
        sessionIDs.append((unsigned int)IDstr.toInt());
    }
    qSort(sessionIDs.begin(), sessionIDs.end());
    Session.store(sessionIDs.last());
}

/// Get the list of all protocol files
void Gui::ListProtocoles()
{
    // List the protocoles shared folder
    paramMut.lock();
    QDir root = QDir(dataFolder + "/Protocoles");
    paramMut.unlock();
    QStringList ProtocolesList_bfr = root.entryList();
    ProtocolesList.clear();
    for (int i = 0; i < ProtocolesList_bfr.count(); i++)
    {
        if  ((ProtocolesList_bfr[i].count())>3 && (QString::compare(ProtocolesList_bfr[i].right(3), "ini") == 0))
        {
            ProtocolesList.append(ProtocolesList_bfr[i]);
        }
    }
}

/// Close the GUI and stop streaming of spikes by the NSP plugin
void Gui::closeEvent (QCloseEvent )
{
    // Close the GUI
    cbSdkSetComment(0, 0, 0, "StopPlugin");
}
