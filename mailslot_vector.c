#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "mailslot_vector.h"

#include "mailslot_vector.h"
#include "mailslot.h"

#include "macro.h"

struct semaphore *vector_lock[MINORS];

#define LOCK_MUTEX(device_instance) down(vector_lock[device_instance])
#define RELEASE_MUTEX(device_instance) up(vector_lock[device_instance])

struct mailslot_vector_s *create_new_mailslot_vector()
{
    struct mailslot_vector_s *mailslots = kmalloc(sizeof(struct mailslot_vector_s), GFP_KERNEL);
    int i;
    if (mailslots == NULL)
    {
        printk("kmalloc failed while allocating the mailslot vector\n");
        goto end;
    }
    mailslots->number_of_instances = 0;
    mailslots->next = 0;
    memset(mailslots->mailslot_instances, 0, MINORS);

    printk("Initializing the semaphores");
    for (i = 0; i < MINORS; i++)
    {
        printk("semaphore number %d done", i);
        vector_lock[i] = kmalloc(sizeof(struct semaphore), GFP_KERNEL);
        sema_init(vector_lock[i], 1);
    }

end:
    return mailslots;
}

bool_t insert_new_mailslot(struct mailslot_vector_s *mailslot_vector, int device_instance, struct mailslot_s *mailslot)
{
    bool_t inserted = FALSE;

    LOCK_MUTEX(device_instance);
    if (mailslot_vector->mailslot_instances[device_instance] == NULL)
    {
        mailslot_vector->mailslot_instances[device_instance] = mailslot;
        mailslot_vector->number_of_instances += 1;
        mailslot_vector->next += 1;
        inserted = TRUE;
    }
    RELEASE_MUTEX(device_instance);

    return inserted;
}

void remove_mailslot_instance(mailslot_vector_t mailslot_vector, int device_instance)
{
    // TODO: fix it
    LOCK_MUTEX(device_instance);
    if (mailslot_vector->mailslot_instances[device_instance] != NULL)
    {
        mailslot_vector->mailslot_instances[device_instance] = NULL;
        mailslot_vector->next = device_instance;
        mailslot_vector->number_of_instances -= 1;
    }
    RELEASE_MUTEX(device_instance);
}

mailslot_t get_mailslot(struct mailslot_vector_s *mailslots, int device_instance)
{
    mailslot_t mailslot;
    LOCK_MUTEX(device_instance);
    mailslot = mailslots->mailslot_instances[device_instance];
    RELEASE_MUTEX(device_instance);
    return mailslot;
}