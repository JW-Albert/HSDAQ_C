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

I32 main( void ) {
    HANDLE hHS;
    const char *IPadd = "192.168.9.40";

    bool ret = false;
    I16 chCnt = 2;
    L32 sampleRate = 12800;

    char tmp[128] = {0};
    sprintf(tmp,"%s,9999,10010",IPadd);
    //Step 1: Create a TCP connection with ET-AR400
	printf("Connecting to device...\n");
    hHS = HS_Device_Create(tmp);
	if( hHS == NULL ) {
		printf("Failed to connect to device.\n");
		return 0;
	}

	//changemode(1);

	ret = HS_SetAIScanParam(hHS, chCnt, 0, 0, sampleRate, 0, 0,0);

	if(ret == false)
	{
		printf("Error code 0x%x\r\n",HS_GetLastError());
		return 0;
	}

	//Step 4: Start AI Scan
	ret = HS_StartAIScan(hHS);
	if( ret == false)  {
		printf("Error code 0x%x\r\n",HS_GetLastError());
		return 0;
	}

	WORD BufferStatus = 0;
	#define BUFFERSIZE 1000
	float fdataBuffer[BUFFERSIZE];
	UL32 ulleng = 0;
	U32 remChannel = 0;

	UL32 CH1checkerror = 0;
	UL32 CH7checkerror = 0;
	UL32 otherCHcheckerror = 0;
	bool quit = false;

	//Step 5: while loop to get total received data count from ETARx00
	while( quit == false ) {			

		ret = HS_GetAIBufferStatus(hHS ,&BufferStatus ,&ulleng);
		if( ret == false ) {
			printf("Error code 0x%x\r\n" ,HS_GetLastError());
			break;
		}

		if( BufferStatus >= 2 ) {
			if( BufferStatus >> 1 & 0x1 )
				printf("AI buffer overflow\r\n");	
			else if( BufferStatus >> 2 & 0x1 )
				printf("AI scan stop\r\n");
			else
				printf("other error\r\n");
			break; 
		}

		if (ulleng) {
			UL32 size = (ulleng > BUFFERSIZE) ? BUFFERSIZE : ulleng;
			UL32 readsize = HS_GetAIBuffer(hHS, fdataBuffer, size);

			if (readsize) {
				for (I32 i = 0; i < readsize; i++) {
					// 計算通道號並輸出數據
					if ((remChannel % chCnt) == 0) {
						if (i > 0) {
							printf("\n"); // 換行，開始新一組輸出
						}
						printf("Channel%u: %f", remChannel % chCnt + 1, fdataBuffer[i]);
					} else {
						printf(", Channel%u: %f", remChannel % chCnt + 1, fdataBuffer[i]);
					}
					remChannel++;
				}
				printf("\n"); // 確保輸出每批數據後換行
			} else {
				printf("No data read (x)\n");
			}
		}


		//偵測鍵盤動作
		/*if( kbhit() ) {
			char ch = getchar();
			printf("%c" ,ch);
			//if(getchar()=='q' || getchar()=='Q')
			if(ch=='q' || ch=='Q')
				//goto END;
				break;									
		}*/        						
	}
	//Step6: Stop AI Scan
	HS_StopAIScan(hHS);
	printf("Stop Scan\r\n");

	//Step7: Close the connection
	HS_Device_Release(hHS);
	printf("Device released\r\n");

	return 0;
}