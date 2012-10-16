//Get software version
#include <stdio.h>
#include <windows.h>

#define I_Code_1	1
#define Tag_it_HF	2
#define ISO_15693	8
#define I_Code_EPC	64
#define I_Code_UID	128

#define Philips_I_Code_1			0x00
#define Texas_Instruments_Tag_it_HF 0x01
#define ISO15693_Tags				0x03
#define Philips_I_Code_EPC			0x06
#define Philips_I_Code_UID			0x07

unsigned short Emb_Crc(void *);

int main()
{
	HANDLE hComm;
	char caString[] = {"Hi"};
	DWORD dwWrite;
	DWORD dwRead;
	DCB	sPStatus;	//	씨리얼 포트 상태 저장. 속도 및 기타.
	COMMTIMEOUTS cType;

    unsigned char msg[128] = {0x16, /* Amount of byte */
							  0x00, 0xb0,
                              0x24, /* 4 */
                              0x01, /* MODE */
                              0xe0, 0x04, 0x01, 0x00, 0x2e, 0xb3, 0x99, 0x58, /*UID*/
                              0x0a, /* DB-ADR */
                              0x01, /* DB-N */
                              0x04, /* DB-SIZE */
                              0x12, 0x34, 0x56, 0x78, /* DB */
                              0xff, 0xff,};
	int iCnt;
	hComm = CreateFile("COM1", 
                           GENERIC_READ | GENERIC_WRITE, 
                           0, 
                           NULL, 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           0);
        
	if(INVALID_HANDLE_VALUE == hComm)
	{
		printf("Failed to open the port\n");
		return 0;
	}
	printf("The port is opened\n");	
	
	if(0 == SetupComm(hComm, 4096, 4096))
	{
		printf("Buffer setting error.\n");	
		CloseHandle(hComm);		
		return 0;
	}
	
	if(0 == PurgeComm(hComm, PURGE_TXABORT | PURGE_TXCLEAR))
	{
		printf("버퍼 초기화 에러 에러.\n");	
		CloseHandle(hComm);		
		return 0;
	}
	sPStatus.DCBlength = sizeof(sPStatus); 
	
	if(0 == GetCommState(hComm, &sPStatus))	/*통신포트의 정보가 들어옴*/
	{
		printf("포트상태 읽기 에러.\n");	
		CloseHandle(hComm);		
		return 0;
	}
	sPStatus.BaudRate = CBR_38400;
	sPStatus.ByteSize = 8;	/*전송 비트 설정*/
	sPStatus.Parity = EVENPARITY;
	sPStatus.StopBits = ONESTOPBIT;
	
	if(0 == SetCommState(hComm, &sPStatus))	/*포트상태 쓰기 에러 검사*/
	{
		printf("포트상태 쓰기 에러.\n");	
		CloseHandle(hComm);		
		return 0;
	}
	
	for(iCnt = 0; *msg > iCnt; iCnt++)
	{
		printf("%02X ", iCnt+1);
	}
	putchar('\n');
	for(iCnt = 0; *msg > iCnt; iCnt++)
	{
		printf("%02X ", msg[iCnt]);
	}
	putchar('\n');
	putchar('\n');
	(*(unsigned short *)(msg + msg[0] - 2)) = Emb_Crc(msg);

	for(iCnt = 0; *msg > iCnt; iCnt++)
	{
		printf("%02X ", iCnt+1);
	}
	putchar('\n');
	
	for(iCnt = 0; *msg > iCnt; iCnt++)
	{
		printf("%02X ", msg[iCnt]);
	}
	putchar('\n');
	
	WriteFile(hComm, msg, msg[0], &dwWrite, 0);
	
	ReadFile(hComm, msg, 1, &dwRead, 0);	/*시리얼 포트, 어디에 읽어들일지, 읽어들이는곳의 크기,*/ 
	ReadFile(hComm, msg + 1, msg[0] - 1, &dwRead, 0);
	
	printf("Status :: ");
	printf("[%02X], %s\n ",msg[3], msg[3] ? "Error" : "Succeed");

	CloseHandle(hComm);		
	printf("The port is closed.\n");	

	return 0;
}

unsigned short Emb_Crc(void *arg)
{
  unsigned short i;
  unsigned short j;
  unsigned short cnt;
  unsigned short crc=0xFFFF;
  unsigned short temp;
  unsigned char *buf = arg;

  cnt = (*buf) - 2;	// crc를 제외

  for(i=0 ; i < cnt ; ++i)
  {
    crc^= *(buf + i);
    for(j=0 ; j < 8 ; ++j)
    {
      if(0 != (crc&0x0001))
      {
        crc=(crc>>1)^0x8408;
      }
      else
      {
        crc=(crc>>1);
      }
    }
  }

  return crc;
}

