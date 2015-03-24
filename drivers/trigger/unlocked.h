#ifndef _UNLOCKED_H_
#define _UNLOCKED_H_

#define SHAKE_HAND		0x00
#define CHECK_PWD		0x01
#define SET_TIME		0x03
#define LOAD_END		0x0e
#define SYSTEM_UNLOCK		0x24

#define DOWN_RECV_BUFFLEN		256
#define DOWN_SEND_BUFFLEN		256

int s_WaitForHostCmd(void);

#endif

