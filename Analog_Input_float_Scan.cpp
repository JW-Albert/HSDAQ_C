// Analog_Input_Scan.cpp : Defines the entry point for the console application.
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "windows2linux.h"
#include <termios.h> //#include <conio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include "./hsdaql.h"

void changemode( int dir ) {
	static struct termios oldt, newt;

	if ( dir == 1 ) {
		tcgetattr( STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt);
	}
	else {
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	}
}

int kbhit ( void ) {
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET (STDIN_FILENO, &rdfs);

	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);
}            

UL32 GetTickCount();
I32 msleep(L32 msec);
#define Sleep msleep

I32 main( I32 argc, char *argv[] ) {
	HANDLE hHS;
    char *IPadd=NULL;
	char sdk_version[16]={0};
	char fw_version[32]={0};

	bool ret= false;
	I16 chCnt=0;
	I16  useGain=0;


	I16 extriggMode=0;
	bool continuousMode=false;
	L32 continuousTimeout=0;
	UL32 targetCnt=0;
	L32 sampleRate=0;

	//UL32 totalRead=0,totalReceived=0;
	I16 DataTransMethod=0;
	I16 AutoRun=0;


	if( argc > 1 ) {
		printf("%d,%s\r\n",argc,argv[1]);
        IPadd=argv[1];
	} else {
		printf("error argument\r\n");
		return 0;
	}
	
	char tmp[128] = {0};
	sprintf(tmp,"%s,9999,10010",IPadd);
	//Step 1: Create a TCP connection with ET-AR400
	hHS = HS_Device_Create(tmp);

	Sleep(10);
	if( hHS != NULL ) {
		printf("ET-ARx00 Analog Input float Scan Testing [continue mode & Software trigger]\r\n");
		HS_GetSDKVersion(sdk_version);
		printf("HSDAQ SDK Ver=%s\r\n",sdk_version);

		HS_GetFirmwareVersion(hHS,fw_version);
		printf("ETARx00 Firmware Ver=%s\r\n",fw_version);
		
        changemode(1);

		chCnt = 4;
		extriggMode = 0;
		targetCnt = 0;
		sampleRate = 20000; //must be <30K Hz in continue mode
		ret = HS_SetAIScanParam(hHS, chCnt, useGain, extriggMode, sampleRate, targetCnt, DataTransMethod,AutoRun);
		//Step 2: SetScanParam sets chCnt, useGain, triggerMode, sampleRate, targetCnt to ETARx00
            /* For details, please refer to the API manual
             * AI CH input Range : 1 ~ 4
             * Gain Range        : 0 ~ 9
             * Mode Range        : 0 ~ 1
             * Sample rate       : 20 ~ 60K (continue mode)
             * targetcnt         : 0 (continue mode)
            */
		if(ret==false)
		{
			printf("Error code 0x%x\r\n",HS_GetLastError());
		}

		HS_GetAIScanParam(hHS, &chCnt, &useGain, &extriggMode, &sampleRate, &targetCnt,&DataTransMethod,&AutoRun); 
		//Step 3: GetScanParam getsx chCnt, useGain, triggerMode, sampleRate, targetCnt to check with ETARx00
		printf("ET-ARx00 first Scan parameters chCnt %d, useGain %d, triggMode %d, sampleRate %ld, targetCnt %lu \n", chCnt, useGain, extriggMode, sampleRate, targetCnt);

		Sleep(10);

		//Step 4: Start AI Scan
		ret=HS_StartAIScan(hHS);
		if( ret == false)  {
			printf("Error code 0x%x\r\n",HS_GetLastError());
		}

		WORD BufferStatus=0;
		#define BUFFERSIZE 1000
		float fdataBuffer[BUFFERSIZE];
		UL32 ulleng=0;
		UL32 totalRecv=0;
		U32 remChannel=0;

		UL32 CH1checkerror=0;
		UL32 CH7checkerror=0;
		UL32 otherCHcheckerror=0;
		bool quit=false;

		//Step 5: while loop to get total received data count from ETARx00
		while( quit == false ) {			

			ret = HS_GetAIBufferStatus(hHS,&BufferStatus,&ulleng);
			if( ret == false ) {
				printf("Error code 0x%x\r\n",HS_GetLastError());
			} else {
				if( BufferStatus >= 2 ) {
					if( BufferStatus >> 1 & 0x1 )
						printf("AI buffer overflow\r\n");	
					else if( BufferStatus >> 2 & 0x1 )
						printf("AI scan stop\r\n");
					else
						printf("other error\r\n");
					break; 
				}
				if( ulleng ) {
					UL32 size=ulleng;
					UL32 temp=0;
					UL32 readsize=0;		
	
					{
						if( size > BUFFERSIZE )
							size = BUFFERSIZE;
						readsize = HS_GetAIBuffer(hHS,fdataBuffer,size);											

						if( readsize ) {				
							printf(".");
							I32 i=0;
	
							for(i = 0 ;i < readsize ;i++) {
								switch(remChannel%chCnt) {	//check data for each channel
									case 1: //1.89V
									case 3:
									/*	if(fdataBuffer[i]>1.35 || fdataBuffer[i]<0.95)
										{
											CH1checkerror++;
											printf("CH1 error %lu, %f\r\n",CH1checkerror,fdataBuffer[i]);
											quit=true;											
										}*/
										break;
									case 7: //0V
										/*if(fdataBuffer[i]>0.2 || fdataBuffer[i]<-0.2)
										{
											CH7checkerror++;
											printf("CH7 error %lu, %f\r\n",CH7checkerror,fdataBuffer[i]);
											quit=true;
										}*/
										break;
								}
								remChannel++;
							}

						}
						else
							printf("x");

						totalRecv = totalRecv + readsize;
					}
				}
			}
			if(targetCnt && totalRecv >= targetCnt) {
				break;
			}

			//continue mode 
			if( kbhit() ) {
                char ch = getchar();
                printf("%c" ,ch);
				//if(getchar()=='q' || getchar()=='Q')
                if(ch=='q' || ch=='Q')
                    //goto END;
					break;									
			}            						
		}
		//Step6: Stop AI Scan
		HS_StopAIScan(hHS);

		printf("AP total read %lu\n", totalRecv);
		//Step7: Close the connection
		HS_Device_Release(hHS);
	}
	return 0;

}