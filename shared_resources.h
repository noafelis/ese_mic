#ifndef SHARED_RESOURCES_H_
#define SHARED_RESOURCES_H_

#include <ti/sysbios/knl/Mailbox.h>


#define NUMMSGS         5

/*
 * This type is accessed by the application. When changing the data members of
 * this structure, considerations should be made for padding and data alignment.
 */
typedef struct MsgObj
{
	Int id;
	Char val;
} MsgObj;

/*
 * Mailbox messages are stored in a queue that requires a header in front of
 * each message. Mailbox_MbxElem is defined such that the header and its size
 * are factored into the total data size requirement for a mailbox instance.
 * Because Mailbox_MbxElem contains Int data types, padding may be added to
 * this struct depending on the data members defined in MsgObj.
 */
typedef struct MailboxMsgObj
{
	Mailbox_MbxElem elem; /* Mailbox header        */
	MsgObj obj; /* Application's mailbox */
} MailboxMsgObj;

/* This buffer is not directly accessed by the application */
extern MailboxMsgObj mailboxBuffer[NUMMSGS];

extern Mailbox_Struct mbxStruct;
extern Mailbox_Handle mbxHandle;

#endif /* SHARED_RESOURCES_H_ */
