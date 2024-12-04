#ifndef _HSDAQ_L_H_
#define _HSDAQ_L_H_

#if defined HSDAQ_EXPORTS
    #if defined WIN32
        #define HS_API(RetType) extern "C" __declspec(dllexport) RetType
    #else
        #define HS_API(RetType) extern "C" RetType __attribute__((visibility("default")))
    #endif
#else
    #if defined WIN32
        #define HS_API(RetType) extern "C" __declspec(dllimport) RetType
    #else
        #define HS_API(RetType) extern "C" RetType
    #endif
#endif



#pragma once             
#pragma warning( disable : 4996 )      

#ifdef	__cplusplus  // for C++ compile use
extern "C" {
#endif	/*	__cplusplus	*/

typedef char *LPSTR;
typedef char *LPTSTR;
typedef const char *LPCSTR;
                
           
//#define U8 uint8_t
#define I16 short  
#define U16 unsigned short
#define I32 int
#define U32 unsigned int    
#define L32 int
#define UL32 unsigned int 
#define F32 float  
#define F64 double  

//typedef uint16_t    WORD;
//typedef uint32_t    DWORD;
#define WORD  U16
#define DWORD UL32  
                     
/*
typedef uint8_t            U8;
typedef int16_t            I16;
typedef uint16_t           U16;
typedef int32_t            I32;
typedef uint32_t           U32;
typedef int64_t            I64;
typedef uint64_t           U64;  
typedef float              F32;
typedef double             F64;*/   

//=========================================================================
// 01. System API
//=========================================================================
HS_API(void) HS_GetSDKVersion(LPSTR sdk_version);
HS_API(HANDLE) HS_Device_Create(LPCSTR ConnectionString);
HS_API(bool) HS_Device_Release(HANDLE obj);
HS_API(bool) HS_GetModelName(HANDLE obj,LPSTR Model_Name); //New 2020 02 26 Edward
HS_API(bool) HS_GetFirmwareVersion(HANDLE obj,LPSTR version);
HS_API(bool) HS_GetHWFirmwareVersion(HANDLE obj,LPSTR fpga_version);
HS_API(bool) HS_Reboot(HANDLE obj);

//=========================================================================
// 02. Configration API
//=========================================================================
HS_API(bool) HS_GetConfig(HANDLE obj,I32 configtype, I32 param, L32 *settingval);
HS_API(bool) HS_GetConfigString(HANDLE obj,I32 configtype, I32 param, char *settingstr, I32 *maxstr);
HS_API(bool) HS_SetConfig(HANDLE obj,I32 configtype, I32 param, L32 settingval);
HS_API(bool) HS_SetConfigString(HANDLE obj,I32 configtype, I32 param, char *settingstr, I32 *maxstr);
HS_API(bool) HS_ReadGainOffset(HANDLE obj,I32 ch,I32 gain,U16 *gainVal,I16 *offsetVal);
//configtype definition
#define BOARD_CONFIG		  0
#define IO_CONFIG			  1
#define HSDAQ_CONFIG		  2
#define DATALOG_CONFIG		  4
#define DATA_RESPONSE_CONFIG  5
#define RMS_EXT_DEVICE_GAIN	  6

//param definition for DATALOG_CONFIG
#define LOGFOLDERTYPE		1
#define LOGFILEMAXSIZE		2

//param definition for DATA_RESPONSE_CONFIG
#define RMS_SOURCE_BASE  0
#define RMS_TRANSFER_RATE  1
#define DATA_TIMEOUT  2

//param definition for HSDAQ_CONFIG
#define HSDAQ_CONNECT_TIMEOUT	1
#define AI_FILTER_AVERAGING		2


//=========================================================================
// 03. IO API
//=========================================================================
HS_API(bool) HS_ReadAI(HANDLE obj,I32 ChannelIndex,I32 gain,float *ai);
HS_API(bool) HS_ReadAIALL(HANDLE obj,I32 gain,float ai[],I32 totalchannel);

HS_API(bool) HS_ReadAIHEX(HANDLE obj,I32 ChannelIndex,I32 gain,L32 *ai);
HS_API(bool) HS_ReadAIALLHEX(HANDLE obj,I32 gain,L32 ai[],I32 totalchannel);//New 2020 02 26 Edward

HS_API(bool) HS_WriteAO(HANDLE obj,I32 ch,I32 gain,float aoval);//New 2020 06 03 Edward
HS_API(bool) HS_WriteAOHEX(HANDLE obj,I32 ch,I32 gain,L32 aoval);//New 2020 06 03 Edward

HS_API(bool) HS_ReadDIO(HANDLE obj,UL32 *diVal, UL32 *doVal);
HS_API(bool) HS_WriteDO(HANDLE obj, UL32  val);
HS_API(bool) HS_WriteDOBit(HANDLE obj, I32 ChannelIndex, bool val);

HS_API(bool) HS_GetEncoderMode(HANDLE obj,I32 ChannelIndex,I32 *Mode,I32 *LPF,I32 *Xor);//New 2020 04 16 Edward
HS_API(bool) HS_SetEncoderMode(HANDLE obj,I32 ChannelIndex,I32 Mode,I32 LPF,I32 Xor);//New 2020 04 16 Edward
HS_API(bool) HS_ReadEncoder(HANDLE obj,I32 ChannelIndex, UL32 *val);//New 2020 04 16 Edward
HS_API(bool) HS_ClearEncoder(HANDLE obj,I32 ChannelIndex);//New 2020 04 16 Edward

HS_API(bool) HS_Calibrate_Data_HEX(HANDLE obj,I32 ch,I32 gain,L32 raw,L32 *Val);
HS_API(bool) HS_Calibrate_Data_Float(HANDLE obj,I32 ch,I32 gain,L32 raw,float *Val);

//=========================================================================
// 03-2 Counter Functions
//=========================================================================
HS_API(bool) HS_SetDICNTConfig(HANDLE obj,DWORD wChannel,DWORD wMode,DWORD dwValue,DWORD reserved);
HS_API(bool) HS_SetCounterConfig(HANDLE obj,DWORD wChannel,DWORD wMode,DWORD dwValue,DWORD reserved);
HS_API(bool) HS_GetDICNTConfig(HANDLE obj,DWORD wChannel,DWORD *wMode,DWORD* dwValue, DWORD *reserved);
HS_API(bool) HS_GetCounterConfig(HANDLE obj, DWORD wChannel, DWORD *wMode, DWORD* dwValue, DWORD *reserved);
HS_API(bool) HS_GetCounter(HANDLE obj,DWORD wChannel, DWORD *dwValue);
HS_API(bool) HS_GetDICNT(HANDLE obj,DWORD wChannel, DWORD *dwValue);
HS_API(bool) HS_GetCounterAll(HANDLE obj,DWORD dwValue[],I32 totalchannel);
HS_API(bool) HS_GetDICNTAll(HANDLE obj,DWORD dwValue[],I32 totalchannel);
HS_API(bool) HS_ClearCounter(HANDLE obj,DWORD wChannel);
HS_API(bool) HS_ClearDICNT(HANDLE obj,DWORD wChannel);
HS_API(bool) HS_ClearCounterALL(HANDLE obj);
HS_API(bool) HS_ClearDICNTALL(HANDLE obj);

#define CNT_DISABLE 0
#define CNT_ENABLE  1
#define CNT_SYNC	2
//=========================================================================
// 04. High speed DAQ API
//=========================================================================
HS_API(bool) HS_SetAIScanParam(HANDLE obj,I16 pacerChCnt, I16 pacerGain, I16 triggerMode, L32 sampleRate, UL32 targetCnt, I16 DataTransMethod, I16 AutoRun);
HS_API(bool) HS_GetAIScanParam(HANDLE obj, I16 *pacerChCnt, I16 *pacerGain, I16 *triggerMode, L32 *sampleRate, UL32 *targetCnt, I16 *DataTransMethod, I16 *AutoRun);
HS_API(bool) HS_GetAIBufferStatus(HANDLE obj,WORD *wBufferStatus, DWORD *dwDataCountOnBuffer);
HS_API(bool) HS_SetAIAnalogTriggerParam(HANDLE obj,I32 analogmode,char En_Channel[],float hightriglevel[],float lowtriglevel[],I32 totalSetchannel,UL32 leftsidecnt,UL32 rightsidecnt,UL32 RESERVED);
HS_API(bool) HS_GetAIAnalogTriggerParam(HANDLE obj,I32 *analogmode,char En_Channel[],float hightriglevel[],float lowtriglevel[],I32 totalGetchannel,UL32 *leftsidecnt,UL32 *rightsidecnt,UL32 *RESERVED);
HS_API(bool) HS_SetAIDelayTriggerParam(HANDLE obj, UL32 delaytime,UL32 RESERVED);
HS_API(bool) HS_GetAIDelayTriggerParam(HANDLE obj, UL32* delaytime,UL32* RESERVED);

HS_API(DWORD) HS_GetAIBufferHex(HANDLE obj,DWORD *wBuffer, DWORD dwBufferSize);
HS_API(DWORD) HS_GetAIBuffer(HANDLE obj,float *fBuffer, DWORD dwBufferSize);
HS_API(bool) HS_ClearAIBuffer(HANDLE obj);
HS_API(bool) HS_StartAIScan(HANDLE obj);
HS_API(bool) HS_StopAIScan(HANDLE obj);

HS_API(bool) HS_GetTotalSamplingStatus(HANDLE obj,UL32 *totalReadCnt, U32 *SamplingStatus);
HS_API(bool) HS_TransmitDataCmd(HANDLE obj);

HS_API(WORD) HS_SetEventCallback(HANDLE obj, WORD wEventType,WORD EventParam,PVOID CallbackFun,void *pdwCallBackParameter);
HS_API(WORD) HS_RemoveEventCallback(HANDLE obj,WORD wEventType);

enum AI_TRIGGER_TYPE
{ 
 AI_TRI_SOFTWARE=0,  //Software trigger 0
 AI_TRI_EXTERNAL, //External trigger 1
 AI_TRI_POST, //Post-trigger 2
 AI_TRI_PRE,   //Pre-trigger 3
 AI_TRI_MID,   //Middle trigger 4 
 AI_TRI_DELAY, //Delay trigger 5
 AI_TRI_AI,  //Analog input trigger 6   
 AI_CONTINUOUS_TRI_POST, //Post-trigger 9
};
//=========================================================================
// 05. Synchronous Input DAQ API
//=========================================================================
HS_API(bool) HS_SetSyncInScanParam(HANDLE hobj,DWORD SyncInheader,WORD InChNumArray[],WORD InChTypeArray[],WORD Arraycount,DWORD Options,DWORD Reserved);
HS_API(bool) HS_GetSyncInScanParam(HANDLE hobj,DWORD *SyncInheader,WORD InChNumArray[],WORD InChTypeArray[],WORD Arraycount,WORD *ActualArrayAmout,DWORD *Options,DWORD *Reserved);
HS_API(DWORD) HS_GetSyncInBuffer(HANDLE hobj,void *packetheader,void **wfAIBuffer,BYTE **bDIBuffer,BYTE **bDOBuffer,void **pDICNTbuffer,void **pCNTbuffer,void *pUDbuffer1,void *pUDbuffer2,DWORD dwFrameDataNumber); 
HS_API(DWORD) HS_GetSyncInBufferDW(HANDLE hobj,DWORD *packetheader,DWORD **wfAIBuffer,BYTE **bDIBuffer,BYTE **bDOBuffer,DWORD **pDICNTbuffer,DWORD **pCNTbuffer,DWORD **pUDbuffer1,DWORD **pUDbuffer2,DWORD dwFrameDataNumber); 
HS_API(DWORD) HS_GetSyncInBufferLV(HANDLE hobj,DWORD *packetheader,DWORD *wfAIBuffer,BYTE *bDIBuffer,BYTE *bDOBuffer,DWORD *pDICNTbuffer,DWORD *pCNTbuffer,DWORD *pUDbuffer1,DWORD *pUDbuffer2,DWORD dwFrameDataNumber); 

HS_API(bool) HS_GetSyncInBufferStatus(HANDLE hobj,WORD *wBufferStatus,DWORD *dwFrameCountOnBuffer);
HS_API(bool) HS_ClearSyncInBuffer(HANDLE hobj); 
HS_API(bool) HS_GetSyncInTotalSamplingStatus(HANDLE hobj,UL32 * totalReadCnt,U32 * SamplingStatus);

enum SYNC_IN_TYPE
{ 
 SYNC_IN_AI=0,  //2bytes (Hex to float)
 SYNC_IN_AI_HEX, //2bytes
 SYNC_IN_WORD_DI_CNT, //2bytes
 SYNC_IN_WORD_CNT,   //2bytes
 SYNC_IN_DWORD_DI_CNT, //4bytes
 SYNC_IN_DWORD_CNT,  //4bytes
 SYNC_IN_DI,     //One bit represents a channel
 SYNC_IN_DO,	 //One bit represents a channel
 SYNC_IN_UD_BYTE,  //1bytes
 SYNC_IN_UD_WORD,  //2bytes
 SYNC_IN_UD_DWORD, //4bytes
 SYNC_IN_UD_FLOAT //4bytes
};

#define SYNC_DISABLE	0
#define SYNC_ENABE		1

//=========================================================================
// 06. Data logger API
//=========================================================================
HS_API(bool) HS_StartLogger(HANDLE obj,char *filePath,I32 interval,I32 filetype);
HS_API(bool) HS_StartLoggerW(HANDLE obj,TCHAR *filePath,I32 interval,I32 filetype);
HS_API(bool) HS_StopLogger(HANDLE obj);

HS_API(I32) HS_GetAllLogFilesW(TCHAR *folderpath,I32 filetype);
HS_API(I32) HS_GetAllLogFiles(char *folderpath,I32 filetype);
HS_API(HANDLE) HS_LogFile_Open_byIndexW(I32 index,TCHAR *getfullFilename);
HS_API(HANDLE) HS_LogFile_Open_byIndex(I32 index,char *getfullFilename);
HS_API(HANDLE) HS_LogFile_Open(char *fullFilename);
HS_API(HANDLE) HS_LogFile_OpenW(TCHAR *fullFilename);
HS_API(bool) HS_LogFile_Close(HANDLE hobj);
HS_API(bool)HS_GetLogFileInfo(HANDLE hobj,char* name,DWORD *filesize,I32 *filetype,I32 *fileversion);
HS_API(bool)HS_GetLogFile_AIScanConfigInfo(HANDLE hobj,I16 *pacerChCnt,I16 *pacerGain,I16 *triggerMode,L32 *sampleRate,I16 *DataTransMethod,I16 *SyncMode);
HS_API(bool)HS_GetLogFile_GainOffset(HANDLE hobj,I32 ch,I32 gain,U16 *gainVal,I16 *offsetVal);
HS_API(bool)HS_GetLogFile_AIScanSampleInfo(HANDLE hobj,DWORD *sampleCount,char *StartDate,char *StartTime);
HS_API(DWORD)HS_GetLogFile_AIData(HANDLE hobj,I32 StartIndx,DWORD count,float *fAIData); //calibrated data (float) from read .txt
HS_API(DWORD)HS_GetLogFile_AIDataHex(HANDLE hobj,I32 StartIndx,DWORD count,L32 *AIData); //calibrated data (Hex) from .bin

//=========================================================================
// 06.1. SD Files API
//=========================================================================
HS_API(bool)HS_GetSDAllLogFiles(HANDLE hobj,UL32 *count);
HS_API(bool)HS_GetSDLogFile_Info(HANDLE hobj,UL32 idx,DWORD *filesize,DWORD *sampleCount,char *DateTime);
//idx : 0 ~ (count-1)
HS_API(bool)HS_DownloadSDFileW(HANDLE hobj,UL32 idx,char Is_Delete_File,TCHAR *filePath,I32 filetype);
//filetype BIN TXT TSM(?)
HS_API(bool)HS_DownloadAllSDFiles(HANDLE hobj,TCHAR *filePath,I32 filetype);

//=========================================================================
//  07. Error Handling API
//=========================================================================
HS_API(DWORD) HS_GetLastError();
HS_API(void) HS_SetLastError(DWORD errorno);
HS_API(void) HS_ClearLastError();
HS_API(void) HS_GetErrorMessage(DWORD dwMessageID, LPSTR lpBuffer);

//=========================================================================
//  08. Multi I-9012 Firmware API
//=========================================================================
HS_API(bool) HS_Get_Module_Count(HANDLE obj,I32 *cnt, char *slot_arr);
HS_API(bool) HS_Configure_Trig_Out(HANDLE obj,I32 slot,I32 opt);

//=========================================================================
//  09. Software Filter API
//=========================================================================
HS_API(bool) HS_Init_Software_Filter(HANDLE obj,I32 ch,I32 filter_order,L32 sampling);
HS_API(bool) HS_Set_LowPassFilter(HANDLE obj,I32 ch,I32 en,float upper_f);
HS_API(bool) HS_Set_HighPassFilter(HANDLE obj,I32 ch,I32 en,float lower_f);
HS_API(bool) HS_Set_BandPassFilter(HANDLE obj,I32 ch,I32 en,float upper_f,float lower_f);
HS_API(bool) HS_Set_BandStopFilter(HANDLE obj,I32 ch,I32 en,float upper_f,float lower_f);

//=========================================================================
// Error Codes
//=========================================================================
#define  HS_ERR_SUCCESS				0x00000
#define  HS_ERR_UNKNOWN				0x00001
#define  HS_ERR_INVALID_MODEL		0xFFFFF
//=====================================================================
// 0x10000~ 0x12999 system API Error //WSAGetLastError 
//Basic=====================================================================
#define  HS_ERR_BASE							0x13000
#define  HS_ERR_UNKNOWN_MODULE					(HS_ERR_BASE + 3)
#define  HS_ERR_INVALID_MAC						(HS_ERR_BASE + 4)
#define  HS_ERR_FUNCTION_NOT_SUPPORT			(HS_ERR_BASE + 6)
#define  HS_ERR_MODULE_UNEXISTS					(HS_ERR_BASE + 7)
#define  HS_ERR_FUNCTION_REPEAT_CALLED			(HS_ERR_BASE + 9)
#define  HS_ERR_INVALID_HANDLE_VALUE			(HS_ERR_BASE + 10)
#define  HS_ERR_DEVICE_IO_CONTROL				(HS_ERR_BASE + 11)
#define  HS_ERR_INVALID_PARAMETER				(HS_ERR_BASE + 12)
#define  HS_ERR_SDK_LOADING						(HS_ERR_BASE + 13)
#define  HS_ERR_MEMORY_ALLOCATED				(HS_ERR_BASE + 14)

    //Memory Access=============================================================
#define  HS_ERR_MEMORY_BASE						0x14000
#define  HS_ERR_MEMORY_INVALID_SIZE				(HS_ERR_MEMORY_BASE + 8)

   //DATA log=============================================================
#define  HS_ERR_DATALOG_BASE					0x14100		
#define  HS_ERR_DATALOG_INVALID_SIZE	  		(HS_ERR_DATALOG_BASE + 1)
#define  HS_ERR_DATALOG_CONFIGFILE_NOFOUND		(HS_ERR_DATALOG_BASE + 2)

    //Watch Dog=================================================================
#define  HS_ERR_WDT_BASE						0x15000
#define  HS_ERR_WDT_INVALID_VALUE				(HS_ERR_WDT_BASE + 1)
#define  HS_ERR_WDT_UNEXIST						(HS_ERR_WDT_BASE + 2)
#define  HS_ERR_WDT_BASE_NOT_SET				(HS_ERR_WDT_BASE + 3)
#define  HS_ERR_WDT_OS_FOR_OS_STARTUP  			(HS_ERR_WDT_BASE + 4)

    //Device open and close======================================================================
#define  HS_ERR_DEVICE_BASE                     0x17000
#define  HS_ERR_DEVICE_CHECKSUM                 (HS_ERR_DEVICE_BASE+1)
#define  HS_ERR_DEVICE_READ_TIMEOUT             (HS_ERR_DEVICE_BASE+2)
#define  HS_ERR_DEVICE_RESPONSE                 (HS_ERR_DEVICE_BASE+3)
#define  HS_ERR_DEVICE_UNDER_INPUT_RANGE        (HS_ERR_DEVICE_BASE+4)
#define  HS_ERR_DEVICE_EXCEED_INPUT_RANGE       (HS_ERR_DEVICE_BASE+5)
#define  HS_ERR_DEVICE_OPEN_FAILED              (HS_ERR_DEVICE_BASE+6)
#define  HS_ERR_DEVICE_INVALID_VALUE            (HS_ERR_DEVICE_BASE+8)
#define  HS_ERR_DEVICE_INTERNAL_BUFFER_OVERFLOW (HS_ERR_DEVICE_BASE+9)
#define  HS_ERR_DEVICE_SEND                     (HS_ERR_DEVICE_BASE+10)
#define  HS_ERR_DEVICE_DATA_CONNECT             (HS_ERR_DEVICE_BASE+11)

    //IO========================================================================
#define  HS_ERR_IO_BASE							0x18000
#define  HS_ERR_IO_NOT_SUPPORT					(HS_ERR_IO_BASE+1)
#define  HS_ERR_IO_ID							(HS_ERR_IO_BASE+2)
#define  HS_ERR_IO_SLOT							(HS_ERR_IO_BASE+3)
#define  HS_ERR_IO_CHANNEL						(HS_ERR_IO_BASE+4)
#define  HS_ERR_IO_GAIN							(HS_ERR_IO_BASE+5)
#define  HS_ERR_IO_INT_MODE						(HS_ERR_IO_BASE+6)
#define  HS_ERR_IO_VALUE_OUT_OF_RANGE			(HS_ERR_IO_BASE+7)
#define  HS_ERR_IO_CHANNEL_OUT_OF_RANGE			(HS_ERR_IO_BASE+8)
#define  HS_ERR_IO_DO_CANNOT_OVERWRITE			(HS_ERR_IO_BASE+10)
#define  HS_ERR_IO_AO_CANNOT_OVERWRITE			(HS_ERR_IO_BASE+11)
#define  HS_ERR_IO_OPERATION_MODE				(HS_ERR_IO_BASE+12)
#define  HS_ERR_IO_DELAY_TIME					(HS_ERR_IO_BASE+13)
#define  HS_ERR_IO_ANALOG_MODE					(HS_ERR_IO_BASE+14)
#define  HS_ERR_IO_ANALOG_RANGE					(HS_ERR_IO_BASE+15)
#define  HS_ERR_IO_ANALOG_COUNT					(HS_ERR_IO_BASE+16)
#define  HS_ERR_IO_BUSY							(HS_ERR_IO_BASE+17)
    //RMS========================================================================
#define  HS_ERR_RMS_BASE					     0x19000
#define  HS_ERR_RMS_PARAM						(HS_ERR_RMS_BASE+1)
#define  HS_ERR_CMD_SETVAL						(HS_ERR_RMS_BASE+2)

/*callback event*/
#define	EVENT_ERROR					0x0001
#define EVENT_N_SAMPLE_REACH		0x0002
#define EVENT_DATA_SAMPLING_TIMEOUT	0x0004
#define EVENT_LAN_BUFFER_OVERFLOW	0x0008
#define EVENT_LOG_N_SAMPLE_REACH	0x0010


#define __int16 __int16_t
#define __int32 __int32_t
typedef struct daqtime {     //timestamp type 1
	//WORD year;  //16bits
	//char month;
	__int16 day:5;//5bits 
	__int16 hour:5;//5bits 
	__int16 minute:6;//6bits 
	__int16 sec:6;//6bits 
	__int16  msec:10; //10bits
} DAQ_TIME; //total 32bits

typedef struct {     //timestamp type 2     
	__int32 minute:6;//6bits 
	__int32 sec:6;//6bits 
	__int32 msec:10; //10bits
	__int32 usec:10; //10bits
} DAQ_TIME2; //total 32bits	

#ifdef	__cplusplus
}
#endif	/*	__cplusplus	*/

#endif //_HSDAQ_L_H_