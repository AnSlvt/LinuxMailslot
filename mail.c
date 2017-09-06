#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "mailslot.h"
#include "mail.h"

#include "macro.h"

struct msg_obj_s *create_new_msg(const mailslot_t mailslot, const char *msg, int len)
{
    struct msg_obj_s *mail = NULL;

    if (len <= mailslot->current_msg_size)
    {
        mail = kmalloc(sizeof(struct msg_obj_s), GFP_KERNEL);
        mail->msg = kmalloc(len * sizeof(char), GFP_KERNEL);
        copy_from_user(mail->msg, msg, len);
        mail->msg[len - 1] = '\0';
        mail->mailslot = mailslot;
        mail->msg_len = len;
    }

    printk("%s: wrote %s of %d bytes\n", DEVICE_NAME, mail->msg, mail->msg_len);

    return mail;
}

void free_msg(struct msg_obj_s *msg)
{
    kfree(msg->msg);
    kfree(msg);
}

char *get_msg(struct msg_obj_s *mail)
{
    return mail->msg;
}

int get_msg_len(struct msg_obj_s *mail)
{
    return mail->msg_len;
}