#ifndef __MACRO_H
#define __MACRO_H

#define MINORS 256
#define DEFAULT_MAX_MSG 30
#define DEFAULT_MAX_MSG_SIZE 256
#define DEVICE_NAME "mail_slot"

typedef enum { FALSE, TRUE } bool_t;

/* ****** ioctl macros ****** */

#define IOCTL_MAGIC 0xF70000

#define SEQUENCE_CMD_BLOCKING 0x000001
#define SEQUENCE_CMD_NONBLOCKING 0x000010
#define SEQUENCE_CMD_CHANGE_MSG_SIZE 0x000011
#define SEQUENCE_CMD_INCREASE_MAX_MSGS 0x000100
#define SEQUENCE_CMD_DECREASE_MAX_MSGS 0x0000101
#define SEQUENCE_CMD_GET_MAX_MSGS 0x000110

#define BLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_BLOCKING | _IOC_NONE
#define NONBLOCKING_CMD IOCTL_MAGIC | SEQUENCE_CMD_NONBLOCKING | _IOC_NONE
#define CHANGE_MSG_SIZE_CMD IOCTL_MAGIC | SEQUENCE_CMD_CHANGE_MSG_SIZE | _IOC_NONE
#define INCREASE_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_INCREASE_MAX_MSGS | _IOC_NONE
#define DECREASE_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_DECREASE_MAX_MSGS | _IOC_NONE
#define GET_MAX_MSGS_CMD IOCTL_MAGIC | SEQUENCE_CMD_GET_MAX_MSGS | _IOC_NONE

#endif