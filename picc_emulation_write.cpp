#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include "CVAPIV01_DESFire.h"

#define DEVICE_ADDRESS	(0)

//INS preprocessor
#define SELECT 			(0xA4)
#define READ_BINARY		(0xB0)
#define UPDATE_BINARY	(0xD6)

//program mode definition
#define READ_MODE		(true)
#define WRITE_MODE 		(false)

static void
intr_hdlr(int sig)
{
  (void) sig;
  CloseComm();
  _exit(2);
}

void print_data(unsigned char *Data, unsigned char Len, const char *Type)
{
	int i=0;
	bool success = false;
	
	if (!strcmp(Type,"RetData"))
	{
		Len+=1;
		printf("Ret Data:\n");
		success = true;
	}
	else if (!strcmp(Type,"Response"))
	{
		printf("Response Data:\n");
		success = true;
	}
	else if (!strcmp(Type,"NDEF"))
	{
		printf("NDEF Data:\n");
		success = true;
	}
	else if (!strcmp(Type,"Result"))
	{
		for(i=0;i<Len;i++)
		{
			printf("%02X", Data[i]);
		}
		printf("\n");
		success = false;
	}
	

	if (success == true)
	{
		for(i=0;i<Len;i++)
		{
			printf("%02X ", Data[i]);
			if(!((i+1)%8))printf("\n");
		}
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	//~ bool valid_arg = false;
	bool mode = WRITE_MODE;
	
	//~ if (argc == 2)
	//~ {
		//~ if(!strcmp(argv[1],"read"))
		//~ {
			//~ valid_arg = true;
			//~ mode = READ_MODE;
		//~ }
		//~ else if(!strcmp(argv[1],"write"))
		//~ {
			//~ valid_arg = true;
			//~ mode = WRITE_MODE;
		//~ }
		//~ else valid_arg = false;
	//~ }
	//~ 
	//~ if (valid_arg == false)
	//~ {
		//~ printf("\n");
		//~ printf("usage:\n");
		//~ printf("\t");
		//~ printf("./picc_emulation [PARAMETER]\n\n");
		//~ printf("PARAMETER:\n");
		//~ printf("\t");
		//~ printf("read \t: Read NDEF message from NFC Forum Type 4 Tag emulated by the reader to NFC-compliant Smartphone\n");
		//~ printf("\t");
		//~ printf("write \t: Write NDEF message from NFC-compliant Smartphone to NFC Forum Type 4 Tag emulated by the reader\n");
		//~ printf("\n");
		//~ return 0;
	//~ }
	
	//~ signal(SIGINT, intr_hdlr);
	signal(SIGTERM, intr_hdlr);
	
	//Reader connect with USB interface
	CV_SetCommunicationType(1);
	
	int Addr=99;
	int &CurAddr = Addr;
	char SerialNum[8];
	bool open_reader = false;
	int open_count = 0;
	while(!open_reader)
	{
		if(!GetSerialNum(DEVICE_ADDRESS, CurAddr, SerialNum))
		{
			printf("Address: %d, SN: %s\n", Addr, SerialNum);
			open_reader = true;
		}
		else
		{
			if(open_count < 5)
			{
				fprintf(stderr, "fail to initialize reader. retry attempt\n");
			}
			CloseComm();
			CV_SetCommunicationType(1);
			if(open_count >= 5)
			{
				fprintf(stderr, "fail to initialize reader. please reconnect\n");
				_exit(1);
			}
		}
		printf("opencount:%d\n",open_count);
		open_count++;
		usleep(10*1000);
	}
	
	const unsigned char All_Read_Data[0xE9] = {
		0x00,0xE7, //NDEF Length
		0xD1,0x01,0xE3,0x54,0x02,0x65,0x6E, //NDEF Header
		0x50,0x65,0x73,0x61,0x6e,0x20,0x64,0x69,0x6b,0x69,0x72,0x69,0x6d,0x20,0x64,0x61,
		0x72,0x69,0x20,0x43,0x4e,0x33,0x37,0x30,0x20,0x55,0x53,0x42,0x20,0x72,0x65,0x61,
		0x64,0x65,0x72,0x2e,0x20,0x50,0x65,0x73,0x61,0x6e,0x20,0x64,0x69,0x6b,0x69,0x72,
		0x69,0x6d,0x20,0x64,0x65,0x6e,0x67,0x61,0x6e,0x20,0x63,0x61,0x72,0x61,0x20,0x6d,
		0x65,0x6e,0x6a,0x61,0x64,0x69,0x6b,0x61,0x6e,0x20,0x43,0x4e,0x33,0x37,0x30,0x20,
		0x55,0x53,0x42,0x20,0x72,0x65,0x61,0x64,0x65,0x72,0x20,0x73,0x65,0x6f,0x6c,0x61,
		0x68,0x2d,0x6f,0x6c,0x61,0x68,0x20,0x73,0x65,0x62,0x61,0x67,0x61,0x69,0x20,0x4e,
		0x46,0x43,0x20,0x54,0x61,0x67,0x20,0x54,0x79,0x70,0x65,0x20,0x34,0x2e,0x20,0x50,
		0x65,0x73,0x61,0x6e,0x20,0x69,0x6e,0x69,0x20,0x73,0x65,0x6e,0x67,0x61,0x6a,0x61,
		0x20,0x64,0x69,0x62,0x75,0x61,0x74,0x20,0x70,0x61,0x6e,0x6a,0x61,0x6e,0x67,0x20,
		0x75,0x6e,0x74,0x75,0x6b,0x20,0x6d,0x65,0x6d,0x62,0x75,0x6b,0x74,0x69,0x6b,0x61,
		0x6e,0x20,0x62,0x61,0x68,0x77,0x61,0x20,0x70,0x65,0x6e,0x67,0x69,0x72,0x69,0x6d,
		0x61,0x6e,0x20,0x70,0x65,0x73,0x61,0x6e,0x20,0x70,0x61,0x6e,0x6a,0x61,0x6e,0x67,
		0x20,0x64,0x61,0x70,0x61,0x74,0x20,0x64,0x69,0x6c,0x61,0x6b,0x75,0x6b,0x61,0x6e
		//0x2e //NDEF Payload
	};

	const unsigned char All_Write_Data[0x05] = {
		0x00,0x03, //NDEF Length
		0xD0,0x00,0x00 //empty NDEF message
	};

	unsigned char RcvdNDEF[262];
	memset(RcvdNDEF, 0, 262);
	
	unsigned char RcvdNDEFLen = 0;

	unsigned char MParam[6];
	MParam[0] = 0x01;
	MParam[1] = 0x04;
	MParam[2] = 0x06;
	MParam[3] = 0x06;
	MParam[4] = 0x06;
	MParam[5] = 0x20;
	
	unsigned char FParam[18];
	memset(FParam, 0, 18);
	
	unsigned char NFCID3t[10];
	memset(FParam, 0, 10);
	NFCID3t[1] = 0x06;
	NFCID3t[2] = 0x06;
	NFCID3t[3] = 0x06;
	
	unsigned char RetData[262];
	memset(RetData, 0, 262);
	
	unsigned char empty = 0;
	
	unsigned char TgResponse[262];
	memset(TgResponse, 0, 262);
	
	unsigned char TgResLen;
	
	bool PICC_init = false;
	while (!PICC_init)
	{
		usleep(300*1000);
		//~ int NFC_Picc_Init (	int DeviceAddress, unsigned char Mode, unsigned char* MParam,
							//~ unsigned char* FParam, unsigned char* NFCID3t, unsigned char GtLen,
							//~ unsigned char* Gt, unsigned char TkLen, unsigned char* Tk, 
							//~ unsigned char* RetData)
		if(!NFC_Picc_Init(DEVICE_ADDRESS, 0x05, MParam, FParam, NFCID3t, empty, &empty, empty, &empty, RetData))
		{
			if(RetData[0] != 0)
			{
				printf("Init OK!\n");
				print_data(RetData,RetData[0],"RetData");
				PICC_init = true;
			}
			else printf("Init fail!\n");
		}
		else printf("Init func call fail!\n");
		
	}
	
	int i;
	unsigned char INS;

	bool PICC_NDEF_detection = false;
	bool write_complete = false;
	while(!PICC_NDEF_detection)
	{
		memset(RetData, 0, 262);
		memset(TgResponse, 0, 262);
		if(!NFC_Picc_Command(DEVICE_ADDRESS, RetData))
		{
			printf("NFC Picc Command OK!\n");
			print_data(RetData,RetData[0],"RetData");
			
			INS = RetData[3];
			
			switch(INS)
			{
				case SELECT:
				{
					bool Flag = false;
					unsigned char Lc;
					Lc = RetData[6];
					unsigned char DataBytes[Lc];

					if (Lc)
					{
						for(i=0;i<Lc;i++)DataBytes[i]=RetData[7+i];
					}
					
					if (Lc == 7) //NDEF Tag Application Select
					{
						unsigned char CmpData[7] = {0xD2,0x76,0x00,0x00,0x85,0x01,0x01};

						for(i=0;i<7;i++)
						{
							if(DataBytes[i]==CmpData[i])Flag = true;
							else Flag = false;
						}
						
						if(Flag == true) //Type 4 tag ver2.0
						{
							TgResponse[0] = 0x6A;
							TgResponse[1] = 0x82;
							TgResLen = 2;
						}
						else //Type 4 tag ver1.0
						{
							TgResponse[0] = 0x90;
							TgResponse[1] = 0x00;
							TgResLen = 2;
						}	
					}
					else if (Lc == 2)
					{
						//Capability Container & NDEF Select command
						if(DataBytes[0] == 0xE1)
						{
							if(DataBytes[1] == 0x03 || DataBytes[1] == 0x04)
								Flag = true;
							else
								Flag = false;
						}
						else Flag = false;
						
						if(Flag == true)
						{
							TgResponse[0] = 0x90;
							TgResponse[1] = 0x00;
							TgResLen = 2;
						}
						else
						{
							TgResponse[0] = 0x6A;
							TgResponse[1] = 0x82;
							TgResLen = 2;
						}	
					}
					else
					{
						TgResponse[0] = 0x6F;
						TgResponse[1] = 0x00;
						TgResLen = 2;
					}
					break;
				}
				
				case READ_BINARY:
				{
					unsigned char Le;
					Le = RetData[6];
					
					if (Le == 0x0F) //Read binary data from CC file
					{
						//See NFCForum Tech Spec Type 4 Tag 2.0
						//Page 29 (Appendix C.1, Detection of NDEF Message)
						//Slight modification in Max NDEF Size (50 -> 1024)
						unsigned char ResBuff[17] 	= { 0x00,0x0F,0x10,0x00,
														0x3B,0x00,0x34,0x04,
														0x06,0xE1,0x04,0x04,
														0x00,0x00,0x00,0x90,
														0x00 };
						memcpy(TgResponse, ResBuff, 17);
						TgResLen = 17;
					}
					else if (Le == 2) //Read NDEF Length
					{
						if (mode == READ_MODE)
						{
							//0x00E9 = Total NDEF length + 2 byte (for NLEN)
							unsigned char ResBuff[4]	= {	0x00,0xE9,0x90,0x00 };
							memcpy(TgResponse, ResBuff, 4);
							TgResLen = 4;
						}
						else
						{
							//0x0005 = Total NDEF length + 2 byte (for NLEN)
							unsigned char ResBuff[4]	= {	0x00,0x05,0x90,0x00 };
							memcpy(TgResponse, ResBuff, 4);
							TgResLen = 4;
						}
					}
					else
					{
						if (Le)
						{
							unsigned char P2;
							P2 = RetData[5];
							if (mode == READ_MODE)
							{
								memcpy(TgResponse, All_Read_Data+P2, Le);
							}
							else
							{
								memcpy(TgResponse, All_Write_Data+P2, Le);
							}
							
							unsigned char SW1SW2[2] = {0x90,0x00};
							memcpy(TgResponse+Le, SW1SW2, 2);
							TgResLen = Le+2;
						}
						else
						{
							TgResponse[0] = 0x6F;
							TgResponse[1] = 0x00;
							TgResLen = 2;
						}
					}
					break;
				}
				
				case UPDATE_BINARY:
				{
					unsigned char Lc = RetData[6];
					if(Lc > 2)
					{
						RcvdNDEFLen = Lc-2;
						for(i=0;i<(Lc-2);i++)RcvdNDEF[i] = RetData[9+i];
						print_data(RcvdNDEF, Lc-2, "NDEF");
					}
					
					TgResponse[0] = 0x90;
					TgResponse[1] = 0x00;
					TgResLen = 2;
					
					write_complete = true;
					
					break;
				}
				
				default:
					TgResponse[0] = 0x6F;
					TgResponse[1] = 0x00;
					TgResLen = 2;
					break;
			}
			
			if(!NFC_Picc_Response(DEVICE_ADDRESS, TgResLen, TgResponse, RetData))
			{
				printf("NFC Picc Response OK!\n");
				print_data(TgResponse,TgResLen,"Response");
				printf("\n");
			}
			else
			{
				printf("NFC Picc Response Fail!\n");
				_exit(3);
			}
		}
		else
		{
			printf("NFC Picc Command Fail!\n");
			PICC_NDEF_detection = true;
		}
	}
	
	if (write_complete)
	{
		printf("DATA:");
		print_data(RcvdNDEF, RcvdNDEFLen, "Result");
		_exit(0);
	}
	
	_exit(4);
}
