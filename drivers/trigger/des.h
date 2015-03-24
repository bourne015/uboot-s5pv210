#ifndef _DES_H
#define _DES_H

#define ENCRYPT	1	/* MODE == encrypt */
#define DECRYPT	0	/* MODE == decrypt */

void Des(unsigned char *input,unsigned char *key,unsigned char *output,int mode);

#endif
