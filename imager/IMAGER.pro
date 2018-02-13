#-------------------------------------------------
#
# Project created by QtCreator 2016-09-05T10:33:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMAGER
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    fileio.cpp \
    imagegrab.cpp \
    nsp.cpp \
    livethread.cpp

HEADERS  += mainwindow.h \
    _GenICamVersion.h \
    cbhwlib.h \
    cbsdk.h \
    fileio.h \
    GenICam.h \
    GenICamFwd.h \
    GenICamVersion.h \
    imagegrab.h \
    nsp.h \
    base/GCArray.h \
    base/GCBase.h \
    base/GCError.h \
    base/GCException.h \
    base/GCLinkage.h \
    base/GCNamespace.h \
    base/GCRTSSUtilities.h \
    base/GCString.h \
    base/GCStringVector.h \
    base/GCSynch.h \
    base/GCTypes.h \
    base/GCUtilities.h \
    clprotocol/CLAllAdapter.h \
    clprotocol/ClAllSerial.h \
    clprotocol/CLAutoBuffer.h \
    clprotocol/CLException.h \
    clprotocol/CLPort.h \
    clprotocol/CLProtocol.h \
    clprotocol/CLProtocolLinkage.h \
    clprotocol/ClSerial.h \
    clprotocol/ClSerialTypes.h \
    clprotocol/CVersion.h \
    clprotocol/DeviceID.h \
    clprotocol/ISerial.h \
    clprotocol/ISerialAdapter.h \
    clprotocol/ISerialAdapterCStyle.h \
    clprotocol/XMLID.h \
    genapi/Autovector.h \
    genapi/ChunkAdapter.h \
    genapi/ChunkAdapterDcam.h \
    genapi/ChunkAdapterGeneric.h \
    genapi/ChunkAdapterGEV.h \
    genapi/ChunkAdapterU3V.h \
    genapi/ChunkPort.h \
    genapi/Compatibility.h \
    genapi/Container.h \
    genapi/Counter.h \
    genapi/DLLLoad.h \
    genapi/EnumClasses.h \
    genapi/EventAdapter.h \
    genapi/EventAdapter1394.h \
    genapi/EventAdapterGeneric.h \
    genapi/EventAdapterGEV.h \
    genapi/EventAdapterU3V.h \
    genapi/EventPort.h \
    genapi/Filestream.h \
    genapi/GenApi.h \
    genapi/GenApiDll.h \
    genapi/GenApiLinkage.h \
    genapi/GenApiNamespace.h \
    genapi/GenApiVersion.h \
    genapi/IBase.h \
    genapi/IBoolean.h \
    genapi/ICategory.h \
    genapi/IChunkPort.h \
    genapi/ICommand.h \
    genapi/IDestroy.h \
    genapi/IDeviceInfo.h \
    genapi/IEnumEntry.h \
    genapi/IEnumeration.h \
    genapi/IEnumerationT.h \
    genapi/IFloat.h \
    genapi/IInteger.h \
    genapi/INode.h \
    genapi/INodeMap.h \
    genapi/INodeMapDyn.h \
    genapi/IPort.h \
    genapi/IPortConstruct.h \
    genapi/IPortRecorder.h \
    genapi/IRegister.h \
    genapi/ISelector.h \
    genapi/ISelectorDigit.h \
    genapi/IString.h \
    genapi/IUserData.h \
    genapi/IValue.h \
    genapi/NodeCallback.h \
    genapi/NodeMapFactory.h \
    genapi/NodeMapRef.h \
    genapi/Persistence.h \
    genapi/Pointer.h \
    genapi/PortImpl.h \
    genapi/PortWriteList.h \
    genapi/Reference.h \
    genapi/RegisterSetHelper.h \
    genapi/SelectorSet.h \
    genapi/StructPort.h \
    genapi/Synch.h \
    genapi/Types.h \
    genapic/GenApiC.h \
    genapic/GenApiC32BitMethods.h \
    genapic/GenApiCDefines.h \
    genapic/GenApiCEnums.h \
    genapic/GenApiCError.h \
    genapic/GenApiCModule.h \
    genapic/GenApiCTypes.h \
    pylon/1394/_Basler1394CameraParams.h \
    pylon/1394/_IIDC1394ChunkData.h \
    pylon/1394/_IIDC1394EventParams.h \
    pylon/1394/_IIDC1394StreamParams.h \
    pylon/1394/Basler1394Camera.h \
    pylon/1394/Basler1394CameraEventHandler.h \
    pylon/1394/Basler1394ConfigurationEventHandler.h \
    pylon/1394/Basler1394DeviceInfo.h \
    pylon/1394/Basler1394GrabResultData.h \
    pylon/1394/Basler1394GrabResultPtr.h \
    pylon/1394/Basler1394ImageEventHandler.h \
    pylon/1394/Basler1394InstantCamera.h \
    pylon/1394/Basler1394InstantCameraArray.h \
    pylon/1394/Pylon1394Camera.h \
    pylon/1394/Pylon1394Includes.h \
    pylon/cameralink/_BaslerCameraLinkCameraParams.h \
    pylon/cameralink/BaslerCameraLinkCamera.h \
    pylon/cameralink/BaslerCameraLinkCameraEventHandler.h \
    pylon/cameralink/BaslerCameraLinkConfigurationEventHandler.h \
    pylon/cameralink/BaslerCameraLinkDeviceInfo.h \
    pylon/cameralink/BaslerCameraLinkGrabResultData.h \
    pylon/cameralink/BaslerCameraLinkGrabResultPtr.h \
    pylon/cameralink/BaslerCameraLinkImageEventHandler.h \
    pylon/cameralink/BaslerCameraLinkInstantCamera.h \
    pylon/cameralink/BaslerCameraLinkInstantCameraArray.h \
    pylon/cameralink/PylonCameraLinkCamera.h \
    pylon/cameralink/PylonCameraLinkIncludes.h \
    pylon/cameralink/PylonCLSerDefs.h \
    pylon/cameralink/PylonCLSerDeviceInfo.h \
    pylon/cameralink/PylonCLSerIncludes.h \
    pylon/gige/_BaslerGigECameraParams.h \
    pylon/gige/_GigEChunkData.h \
    pylon/gige/_GigEEventParams.h \
    pylon/gige/_GigEStreamParams.h \
    pylon/gige/_GigETLParams.h \
    pylon/gige/ActionTriggerConfiguration.h \
    pylon/gige/BaslerGigECamera.h \
    pylon/gige/BaslerGigECameraEventHandler.h \
    pylon/gige/BaslerGigEConfigurationEventHandler.h \
    pylon/gige/BaslerGigEDeviceInfo.h \
    pylon/gige/BaslerGigEGrabResultData.h \
    pylon/gige/BaslerGigEGrabResultPtr.h \
    pylon/gige/BaslerGigEImageEventHandler.h \
    pylon/gige/BaslerGigEInstantCamera.h \
    pylon/gige/BaslerGigEInstantCameraArray.h \
    pylon/gige/GigETransportLayer.h \
    pylon/gige/PylonGigE.h \
    pylon/gige/PylonGigECamera.h \
    pylon/gige/PylonGigEDevice.h \
    pylon/gige/PylonGigEDeviceProxy.h \
    pylon/gige/PylonGigEIncludes.h \
    pylon/private/DeviceSpecificCameraEventHandlerTie.h \
    pylon/private/DeviceSpecificConfigurationEventHandlerTie.h \
    pylon/private/DeviceSpecificGrabResultPtr.h \
    pylon/private/DeviceSpecificImageEventHandlerTie.h \
    pylon/private/DeviceSpecificInstantCamera.h \
    pylon/private/DeviceSpecificInstantCameraArray.h \
    pylon/usb/_BaslerUsbCameraParams.h \
    pylon/usb/_UsbChunkData.h \
    pylon/usb/_UsbEventParams.h \
    pylon/usb/_UsbStreamParams.h \
    pylon/usb/_UsbTLParams.h \
    pylon/usb/BaslerUsbCamera.h \
    pylon/usb/BaslerUsbCameraEventHandler.h \
    pylon/usb/BaslerUsbConfigurationEventHandler.h \
    pylon/usb/BaslerUsbDeviceInfo.h \
    pylon/usb/BaslerUsbGrabResultData.h \
    pylon/usb/BaslerUsbGrabResultPtr.h \
    pylon/usb/BaslerUsbImageEventHandler.h \
    pylon/usb/BaslerUsbInstantCamera.h \
    pylon/usb/BaslerUsbInstantCameraArray.h \
    pylon/usb/PylonUsbCamera.h \
    pylon/usb/PylonUsbDefs.h \
    pylon/usb/PylonUsbIncludes.h \
    pylon/_ImageFormatConverterParams.h \
    pylon/_InstantCameraParams.h \
    pylon/AcquireContinuousConfiguration.h \
    pylon/AcquireSingleFrameConfiguration.h \
    pylon/AviCompressionOptions.h \
    pylon/AviWriter.h \
    pylon/BufferFactory.h \
    pylon/Callback.h \
    pylon/CameraEventHandler.h \
    pylon/ChunkParser.h \
    pylon/ConfigurationEventHandler.h \
    pylon/Container.h \
    pylon/Device.h \
    pylon/DeviceAccessMode.h \
    pylon/DeviceClass.h \
    pylon/DeviceFactory.h \
    pylon/DeviceInfo.h \
    pylon/EventAdapter.h \
    pylon/EventGrabber.h \
    pylon/EventGrabberProxy.h \
    pylon/FeaturePersistence.h \
    pylon/GrabResultData.h \
    pylon/GrabResultPtr.h \
    pylon/Image.h \
    pylon/ImageEventHandler.h \
    pylon/ImageFormat.h \
    pylon/ImageFormatConverter.h \
    pylon/ImagePersistence.h \
    pylon/Info.h \
    pylon/InstantCamera.h \
    pylon/InstantCameraArray.h \
    pylon/InterfaceInfo.h \
    pylon/NodeMapProxy.h \
    pylon/PayloadType.h \
    pylon/Pixel.h \
    pylon/PixelData.h \
    pylon/PixelFormatConverter.h \
    pylon/PixelFormatConverterBayer.h \
    pylon/PixelFormatConverterBayer16.h \
    pylon/PixelFormatConverterGamma.h \
    pylon/PixelFormatConverterGammaPacked.h \
    pylon/PixelFormatConverterMonoPacked.h \
    pylon/PixelFormatConverterMonoXX.h \
    pylon/PixelFormatConverterRGB.h \
    pylon/PixelFormatConverterTruncate.h \
    pylon/PixelFormatConverterTruncatePacked.h \
    pylon/PixelFormatConverterYUV422.h \
    pylon/PixelType.h \
    pylon/PixelTypeMapper.h \
    pylon/Platform.h \
    pylon/PylonBase.h \
    pylon/PylonBitmapImage.h \
    pylon/PylonDeviceProxy.h \
    pylon/PylonGUI.h \
    pylon/PylonGUIIncludes.h \
    pylon/PylonImage.h \
    pylon/PylonImageBase.h \
    pylon/PylonIncludes.h \
    pylon/PylonLinkage.h \
    pylon/PylonUtility.h \
    pylon/PylonUtilityIncludes.h \
    pylon/PylonVersion.h \
    pylon/PylonVersionInfo.h \
    pylon/PylonVersionNumber.h \
    pylon/Result.h \
    pylon/ResultImage.h \
    pylon/ReusableImage.h \
    pylon/SfncVersion.h \
    pylon/SoftwareTriggerConfiguration.h \
    pylon/stdinclude.h \
    pylon/StreamGrabber.h \
    pylon/StreamGrabberProxy.h \
    pylon/ThreadPriority.h \
    pylon/TlFactory.h \
    pylon/TlInfo.h \
    pylon/TransportLayer.h \
    pylon/TypeMappings.h \
    pylon/WaitObject.h \
    pylon/WaitObjects.h \
    pylon/XmlFileProvider.h \
    pylonc/PylonAviCompressionOptions.h \
    pylonc/PylonC.h \
    pylonc/PylonC32BitMethods.h \
    pylonc/PylonCDefines.h \
    pylonc/PylonCEnums.h \
    pylonc/PylonCError.h \
    pylonc/PylonCModule.h \
    livethread.h

FORMS    += mainwindow.ui \
    mainwindow.ui

unix|win32: LIBS += -L$$PWD/x64/ -lGCBase_MD_VC120_v3_0_Basler_pylon_v5_0

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/GCBase_MD_VC120_v3_0_Basler_pylon_v5_0.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libGCBase_MD_VC120_v3_0_Basler_pylon_v5_0.a

unix|win32: LIBS += -L$$PWD/x64/ -lGenApi_MD_VC120_v3_0_Basler_pylon_v5_0

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/GenApi_MD_VC120_v3_0_Basler_pylon_v5_0.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libGenApi_MD_VC120_v3_0_Basler_pylon_v5_0.a

unix|win32: LIBS += -L$$PWD/x64/ -lPylonBase_MD_VC120_v5_0

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/PylonBase_MD_VC120_v5_0.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libPylonBase_MD_VC120_v5_0.a

unix|win32: LIBS += -L$$PWD/x64/ -lPylonC_MD_VC120

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/PylonC_MD_VC120.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libPylonC_MD_VC120.a

unix|win32: LIBS += -L$$PWD/x64/ -lPylonGUI_MD_VC120_v5_0

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/PylonGUI_MD_VC120_v5_0.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libPylonGUI_MD_VC120_v5_0.a

unix|win32: LIBS += -L$$PWD/x64/ -lPylonUtility_MD_VC120_v5_0

INCLUDEPATH += $$PWD/x64
DEPENDPATH += $$PWD/x64

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/x64/PylonUtility_MD_VC120_v5_0.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/x64/libPylonUtility_MD_VC120_v5_0.a

unix|win32: LIBS += -L$$PWD/lib/ -lcbsdkx64

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/cbsdkx64.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/lib/libcbsdkx64.a

DISTFILES +=

RC_ICONS = IMAGER.ico
