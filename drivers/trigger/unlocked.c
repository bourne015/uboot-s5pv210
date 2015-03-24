#include <common.h>
#include <command.h>
#include <malloc.h>
#include "des.h"
#include "unlocked.h"

static unsigned char g_CurRandom[8];

static unsigned char g_DownRecvBuff[DOWN_RECV_BUFFLEN];
static unsigned char g_DownSendBuff[DOWN_SEND_BUFFLEN];

unsigned char CheckPwdStatus=0;
//校验密钥与认证状态：  =0000未成功完成校验与认证；
//                      =0001已完成VOS对PC的校验；
//                      =0010已完成VOS管理密码的校验；
//                      =0100已完成PED解锁密码的校验；
//                      
//

static unsigned char random(void)

{
     unsigned long long rand = get_ticks() * 100000;
     unsigned char a;

     rand = rand * 1664525L + 1013904223L;
     a = rand >> 24; //不是最低8位

       //为了得到0~9, a ~ f, A ~ F 的值
     if (a < 'A')
          a = a % 10 + 48;
    else if (a < 'F')
          a = a % 6 + 65;
     else if (a < 'a' || a > 'f')
          a = a % 6 + 97;

     return a;

}

static void MyRand(unsigned char *buff)
{
	int i;

	for(i=0;i<8;i++)
		buff[i]=random();

	Des(buff, buff, (unsigned char *)"ViewAtPP", 1);
	memcpy(g_CurRandom, buff, 8);
}            

static int SendResp(unsigned char cmd_type, unsigned char *cmd_data, int SndLen)
{       
	char ch, edc, tempbuf[DOWN_SEND_BUFFLEN+6];
	int i, j;     

	j = 0;
	tempbuf[j++] = 0x02;

	tempbuf[j++] = cmd_type;

	edc = cmd_type;
	ch = (unsigned char)(SndLen / 256);
	tempbuf[j++]=ch;

	edc ^= ch;
	ch = (unsigned char)(SndLen % 256);
	edc ^= ch;
	tempbuf[j++] = ch;

	for(i = 0; i <SndLen; i++){    
		tempbuf[j++] = cmd_data[i];
		edc ^= cmd_data[i];
	}
	
	tempbuf[j++] = edc;
#if 0
	for (i=0; i<15; i++){ 
		printf("tempbuf[%d]:%#x\n", i, tempbuf[i]);
	}
#endif
	for (i = 0; i < j; i++){ 	
		putc(tempbuf[i]);
	}
	return 0;
}

static int request_flag = 0;
static int RcvCmd(unsigned char *cmd_type, unsigned char *cmd_data, int *cmd_len)
{
	unsigned char edc;   
	unsigned char tempbuf[10];
	int i,len;

	memset(tempbuf, 0, 10);

	if (!tstc()){
		return -1; 
	}

	tempbuf[0] = (unsigned char)getc();
	if (tempbuf[0] != 0x02){
		if (tempbuf[0] == 'Q'){
			if(request_flag == 0){
				putc('R');
				request_flag = 1;
			}
		}
	return -2;
	}

	edc = 0;
	for (i = 1; i < 4; i++){
		tempbuf[i] = (unsigned char)getc();
		edc ^= tempbuf[i];
	}
	
	*cmd_type = tempbuf[1];
	len = ((int)tempbuf[2])*256 + (int)tempbuf[3];

	for (i=0; i<len+1; i++){  //+1 for edc
		cmd_data[i] = (unsigned char)getc();
		edc ^= cmd_data[i];
	}
	if (edc){
		return -1;
	} 

	*cmd_len = len;
#if 0
	for (i=0; i<10; i++){ 
		printf("tempbuf[%d]:%#x\n", i, tempbuf[i]);
	}
#endif
	return 0;
}

static int ProcessCmd(unsigned char cmd_type, unsigned char *cmd_data, int cmd_len)
{
	int iret;
	unsigned char tempbuf[8];

	if(cmd_type == SHAKE_HAND){
		g_DownSendBuff[0] = 0;
		MyRand(g_DownSendBuff + 1);
		iret = SendResp(cmd_type, g_DownSendBuff, 9);
		return 0;
	} 

	if (!(CheckPwdStatus&0x01) && cmd_type != CHECK_PWD){
		g_DownSendBuff[1] = 0xfe;
		iret = SendResp(cmd_type, g_DownSendBuff, 1); 
		return 0;
	} 

	switch(cmd_type){
		case CHECK_PWD: 
			Des(g_CurRandom, tempbuf, (unsigned char *)"Vpp202OS", 1);
			if (memcmp(cmd_data, tempbuf, 8)){
				g_DownSendBuff[0] = 0x01;
				CheckPwdStatus &= 0xfe;
			}
			else{
				g_DownSendBuff[0] = 0x00;
				CheckPwdStatus |= 0x01;
			}
			
			iret=SendResp(cmd_type, g_DownSendBuff, 1); 
			return 0; 

		case SET_TIME:        
			//SetTime(cmd_data);
			g_DownSendBuff[0] = 0;
			iret=SendResp(cmd_type, g_DownSendBuff, 1); 
			return 0;

		case LOAD_END:
			if(cmd_data[0] == 0){

			}
				//LCD_PRINTXY(0, 4*8, 1, "    下载成功    ", "  Load success. ");
			else{

			}
				//LCD_PRINTXY(0, 4*8, 1, "    下载失败    ", "  Load Failed. ");
			return 1;

		case SYSTEM_UNLOCK:
			//UnlockSystem();
			g_DownSendBuff[0] = 0;
			iret = SendResp(cmd_type, g_DownSendBuff, 1);
			return 0;		  
	}

	return 0;
}

int s_WaitForHostCmd(void)
{
	int   Cmd_Len;
	unsigned char Cmd_Type;
	int Ret;
	unsigned char Flag_Loading;
	int unlocked_successfully = -1;
	Flag_Loading = 0;

	Ret = RcvCmd(&Cmd_Type, g_DownRecvBuff, &Cmd_Len);

	if (Ret == 0) {
		Flag_Loading = 1;

		if (ProcessCmd(Cmd_Type, g_DownRecvBuff, Cmd_Len)){
			Flag_Loading = 0;
			unlocked_successfully = 0;
		}
	}

	return unlocked_successfully;
}

