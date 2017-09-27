#include <asm/uaccess.h>
#include <linux/slab.h>

#include "mailslot.h"
#include "mail.h"

#include "macro.h"

struct msg_obj_s *create_new_msg(const mailslot_t mailslot, const char *msg, int len, int *error)
{
    struct msg_obj_s *mail = NULL;

    if (len <= mailslot->current_msg_size)
    {
        mail = kmalloc(sizeof(struct msg_obj_s), GFP_KERNEL);
        if (mail == NULL)
        {
            printk("%s: error!", DEVICE_NAME);
            *error = -ENOMEM;
            goto null;
        }
        mail->msg = kmalloc(len * sizeof(char), GFP_KERNEL);
        if (mail->msg == NULL)
        {
            kfree(mail);
            printk("%s: error!", DEVICE_NAME);
            *error = -ENOMEM;
            goto null;
        }
        copy_from_user(mail->msg, msg, len);
        mail->msg[len - 1] = '\0';
        mail->mailslot = mailslot;
        mail->msg_len = len;

        printk("%s: wrote %s of %d bytes\n", DEVICE_NAME, mail->msg, mail->msg_len);

        goto end;
    }
    else *error = -E2BIG;

null:
    return NULL;

end:
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