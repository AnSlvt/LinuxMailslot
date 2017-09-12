#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

#include "mailslot_vector.h"
#include "mailslot.h"
#include "mail.h"

#include "macro.h"

#define IS_BLOCKING mailslot->behaviour == BLOCKING

void create_new_mailslot(mailslot_vector_t mailslots, int device_instance)
{
    // Mailslot initialization
    struct mailslot_s *mailslot = kmalloc(sizeof(struct mailslot_s), GFP_KERNEL);
    if (mailslot == NULL)
    {
        printk("kmalloc failed while allocating a new mailslot for instance %d", device_instance);
        return;
    }
    mailslot->current_msg_size = DEFAULT_MAX_MSG_SIZE;
    mailslot->mails = kmalloc(mailslot->current_msg_size * sizeof(struct msg_obj_s), GFP_KERNEL);
    mailslot->next_to_read = 0;
    mailslot->next_to_insert = 0;
    mailslot->empty = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    mailslot->full = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    mailslot->mailslot_sync = kmalloc(sizeof(spinlock_t), GFP_KERNEL);
    mailslot->current_max_msgs = DEFAULT_MAX_MSG;
    mailslot->mails_in = 0;
    mailslot->behaviour = BLOCKING;
    mailslot->open_instances = 1;

    sema_init(mailslot->empty, mailslot->current_max_msgs);
    sema_init(mailslot->full, 0);
    spin_lock_init(mailslot->mailslot_sync);

    // Add the mailslot inside the mailslot vector
    if (!insert_new_mailslot(mailslots, device_instance, mailslot))
    {
        printk("%s: mailslot for %d already exists\n", DEVICE_NAME, device_instance);
        kfree(mailslot->mails);
        kfree(mailslot->empty);
        kfree(mailslot->full);
        kfree(mailslot->mailslot_sync);
        kfree(mailslot);

        mailslot = get_mailslot(mailslots, device_instance);
        mailslot->open_instances += 1;
    }
    else printk("%s: new mailslot created and succesfully inserted in %d\n", DEVICE_NAME, device_instance);
}

void insert_new_msg(struct mailslot_s *mailslot, msg_obj_t msg)
{
    if (IS_BLOCKING) down(mailslot->empty);
    spin_lock(mailslot->mailslot_sync);

    if (!(IS_BLOCKING) && mailslot->mails_in == mailslot->current_max_msgs) goto full;

    mailslot->mails[mailslot->next_to_insert] = msg;
    mailslot->mails_in += 1;
    mailslot->next_to_insert = (mailslot->next_to_insert + 1) % mailslot->current_max_msgs;

full:
    spin_unlock(mailslot->mailslot_sync);
    if (IS_BLOCKING) up(mailslot->full);
}

int read_msg(struct mailslot_s *mailslot, char *buff, int len)
{
    int ret = 0;
    msg_obj_t to_read;

    if (IS_BLOCKING) down(mailslot->full);
    spin_lock(mailslot->mailslot_sync);
    if (!(IS_BLOCKING) && mailslot->mails_in == 0) goto empty;

    to_read = mailslot->mails[mailslot->next_to_read];

    ret = get_msg_len(to_read);
    copy_to_user(buff, get_msg(to_read), ret);

    kfree(to_read);
    mailslot->mails[mailslot->next_to_read] = NULL;
    mailslot->mails_in -= 1;
    mailslot->next_to_read = (mailslot->next_to_read + 1) % mailslot->current_max_msgs;

    printk("%s: read %s of %d bytes\n", DEVICE_NAME, buff, ret);

empty:
    spin_unlock(mailslot->mailslot_sync);
    if (IS_BLOCKING) up(mailslot->empty);
    return ret;
}

void free_mailslot(mailslot_t mailslot)
{
    int i = mailslot->next_to_read;
    int size = mailslot->mails_in;
    for (; i < size; i++)
    {
        free_msg(mailslot->mails[i]);
    }
    kfree(mailslot->mails);
    kfree(mailslot->empty);
    kfree(mailslot->full);
    kfree(mailslot->mailslot_sync);
    kfree(mailslot);
}

void set_behaviour(mailslot_t mailslot, mailslot_behaviour_t new_behaviour)
{
    spin_lock(mailslot->mailslot_sync);
    mailslot->behaviour = new_behaviour;
    spin_unlock(mailslot->mailslot_sync);
}

/*struct msg_obj_s *get_mail(mailslot_t mailslot, int position)
{
    return mailslot->mails[position];
}

int get_mails_in(mailslot_t mailslot)
{
    int size;
    spin_lock(mailslot->mailslot_sync);
    size = mailslot->mails_in;
    spin_unlock(mailslot->mailslot_sync);
    return size;
}

int is_length_compatible(struct mailslot_s *mailslot, int len)
{
    return len <= mailslot->current_msg_size;
}

int there_is_space(struct mailslot_s *mailslot)
{
    int size = get_mails_in(mailslot);
    return size != mailslot->current_max_msgs;
}

int is_empty(struct mailslot_s *mailslot)
{
    int size = get_mails_in(mailslot);
    return size == 0;
}
*/