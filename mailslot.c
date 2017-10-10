#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
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
    mailslot->current_max_msgs = DEFAULT_MAX_MSG;
    mailslot->mails = kmalloc(mailslot->current_max_msgs * sizeof(struct msg_obj_s), GFP_KERNEL);
    mailslot->next_to_read = 0;
    mailslot->next_to_insert = 0;
    mailslot->empty = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    mailslot->full = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    mailslot->mailslot_sync = kmalloc(sizeof(spinlock_t), GFP_KERNEL);
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

int insert_new_msg(struct mailslot_s *mailslot, msg_obj_t msg)
{
    int locked = 0, ret = msg->msg_len;

    if (IS_BLOCKING) down(mailslot->empty);
    else
    {
        locked = down_trylock(mailslot->empty);
        if (locked != 0)
        {
            ret = -ENOSPC;
            goto full;
        }
    }
    spin_lock(mailslot->mailslot_sync);

    mailslot->mails[mailslot->next_to_insert] = msg;
    mailslot->mails_in += 1;
    mailslot->next_to_insert = (mailslot->next_to_insert + 1) % mailslot->current_max_msgs;

    spin_unlock(mailslot->mailslot_sync);

full:
    if (locked == 0) up(mailslot->full);
    return ret;
}

int read_msg(struct mailslot_s *mailslot, char *buff, int len)
{
    int ret = 0;
    int locked = 1;
    msg_obj_t to_read;

    if (IS_BLOCKING) down(mailslot->full);
    else
    {
        locked = down_trylock(mailslot->full);
        if (locked != 0) goto empty;
    }
    spin_lock(mailslot->mailslot_sync);

    to_read = mailslot->mails[mailslot->next_to_read];

    ret = get_msg_len(to_read);
    
    // if the message length is compatible with the read size...
    if (ret <= len)
    {
        mailslot->mails[mailslot->next_to_read] = NULL;
        mailslot->mails_in -= 1;
        mailslot->next_to_read = (mailslot->next_to_read + 1) % mailslot->current_max_msgs;

        printk("%s: read %s of %d bytes\n", DEVICE_NAME, buff, ret);

        spin_unlock(mailslot->mailslot_sync);

        // ... return the message to the user
        copy_to_user(buff, get_msg(to_read), ret);
        kfree(to_read);

        up(mailslot->empty);
    }
    else
    {
        // the requested length is too small, get back to sleep and awake another reader that might have a right request
        spin_unlock(mailslot->mailslot_sync);
        up(mailslot->full);
        ret = -EAGAIN;
    }

empty:
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

void change_msg_max_size(mailslot_t mailslot, int new_size)
{
    spin_lock(mailslot->mailslot_sync);
    mailslot->current_msg_size = new_size;
    spin_unlock(mailslot->mailslot_sync);
}

int increase_max_number_of_msgs(mailslot_t mailslot, int new_size)
{
    int ret = 0, i, old_size;
    msg_obj_t *mails_copy;

    mails_copy = kmalloc(new_size * sizeof(struct msg_obj_s), GFP_KERNEL);
    if (mails_copy == NULL)
    {
        printk("%s: error!", DEVICE_NAME);
        ret = -ENOMEM;
        goto end;
    }

    spin_lock(mailslot->mailslot_sync);

    if (new_size <= mailslot->current_max_msgs)
    {
        ret = -EINVAL;
        kfree(mails_copy);
    }
    else
    {
        // I have more space in the buffer - copy the content of the mailslot in the new buffer
        // and then unlock all the writers that want to use the new space
        old_size = mailslot->current_max_msgs;
        mailslot->current_max_msgs = new_size;

        for (i = mailslot->next_to_read; i < mailslot->next_to_insert; i++)
        {
            mails_copy[i] = mailslot->mails[i];
        }
        kfree(mailslot->mails);
        mailslot->mails = mails_copy;

        for (i = old_size; i < new_size; i++)
            up(mailslot->empty);
    }

    spin_unlock(mailslot->mailslot_sync);

end:
    return ret;
}

int decrease_max_number_of_msgs(mailslot_t mailslot)
{
    int locked, ret = 0, j = 0, i;
    msg_obj_t *mails_copy;

    // reduce the space in the mailslot by one. create a copy is necessary to avoid the loss of messages
    mails_copy = kmalloc((mailslot->current_max_msgs - 1) * sizeof(struct msg_obj_s), GFP_KERNEL);
    if (mails_copy == NULL)
    {
        printk("%s: error!", DEVICE_NAME);
        ret = -ENOMEM;
        goto end;
    }

    spin_lock(mailslot->mailslot_sync);

    // if the lock fails we do not proceed to shrink the space. notify to the user to retry later
    locked = down_trylock(mailslot->empty);
    if (locked != 0)
    {
        ret = -EAGAIN;
        kfree(mails_copy);
        goto retry;
    }

    // handle the circularity of the buffer
    if (mailslot->next_to_insert >= mailslot->next_to_read)
    {
        for (i = mailslot->next_to_read; i < mailslot->next_to_insert; i++)
        {
            mails_copy[j++] = mailslot->mails[i];
        }
    }
    else
    {
        for (i = mailslot->next_to_read; i < mailslot->current_max_msgs; i++)
        {
            mails_copy[j++] = mailslot->mails[i];
        }

        for (i = 0; i < mailslot->next_to_insert; i++)
        {
            mails_copy[j++] = mailslot->mails[i];
        }
    }

    kfree(mailslot->mails);
    mailslot->mails = mails_copy;
    mailslot->next_to_read = 0;
    mailslot->next_to_insert = j;


    mailslot->current_max_msgs = mailslot->current_max_msgs - 1;

retry:
    spin_unlock(mailslot->mailslot_sync);
end:
    return ret;
}

int get_max_number_of_msgs(mailslot_t mailslot)
{
    int ret;
    spin_lock(mailslot->mailslot_sync);
    ret = mailslot->current_max_msgs;
    spin_unlock(mailslot->mailslot_sync);
    return ret;
}