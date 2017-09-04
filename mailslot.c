#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "mailslot_vector.h"
#include "mailslot.h"
#include "mail.h"

#include "macro.h"

struct mailslot_s * create_new_mailslot(mailslot_vector_t mailslots, int device_instance)
{
    // Mailslot initialization
    struct mailslot_s *mailslot = kmalloc(sizeof(struct mailslot_s), GFP_KERNEL);
    if (mailslot == NULL)
    {
        printk("kmalloc failed while allocating a new mailslot for instance %d", device_instance);
        goto end;
    }
    mailslot->current_msg_size = DEFAULT_MAX_MSG_SIZE;
    mailslot->mails = kmalloc(mailslot->current_msg_size * sizeof(struct msg_obj_s), GFP_KERNEL);
    mailslot->instance_position = device_instance;
    mailslot->next_to_read = 0;
    mailslot->next_to_insert = 0;
    mailslot->current_max_msgs = DEFAULT_MAX_MSG;
    mailslot->mails_in = 0;

    // Add the mailslot inside the mailslot vector
    insert_new_mailslot(mailslots, device_instance, mailslot);

end:
    return mailslot;
}

void insert_new_msg(struct mailslot_s *mailslot, msg_obj_t msg)
{
    mailslot->mails[mailslot->next_to_insert] = msg;
    mailslot->mails_in += 1;
    mailslot->next_to_insert = (mailslot->next_to_insert + 1) % mailslot->current_max_msgs;
}

int read_msg(struct mailslot_s *mailslot, char *buff, int len)
{
    int ret = 0;
    msg_obj_t to_read;

    if (is_empty(mailslot)) goto empty;

    to_read = mailslot->mails[mailslot->next_to_read];

    if (len <= to_read->msg_len)
    {
        copy_to_user(buff, get_msg(to_read), len);
        ret = len;
    }
    else
    {
        ret = get_msg_len(to_read);
        copy_to_user(buff, get_msg(to_read), ret);
    }

    kfree(to_read);
    mailslot->mails_in -= 1;
    mailslot->next_to_read = (mailslot->next_to_read + 1) % mailslot->current_max_msgs;

    printk("%s: read %s of %d bytes\n", DEVICE_NAME, buff, ret);

empty:
    return ret;
}

struct msg_obj_s *get_mail(mailslot_t mailslot, int position)
{
    return mailslot->mails[position];
}

int get_mails_in(mailslot_t mailslot)
{
    return mailslot->mails_in;
}

int is_length_compatible(struct mailslot_s *mailslot, int len)
{
    return len <= mailslot->current_msg_size;
}

int there_is_space(struct mailslot_s *mailslot)
{
    return !(mailslot->mails_in == mailslot->current_max_msgs);
}

int is_empty(struct mailslot_s *mailslot)
{
    return mailslot->mails_in == 0;
}