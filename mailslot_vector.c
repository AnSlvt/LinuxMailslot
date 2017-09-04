#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "mailslot_vector.h"

#include "mailslot_vector.h"
#include "mailslot.h"

#include "macro.h"

struct mailslot_vector_s * create_new_mailslot_vector()
{
    struct mailslot_vector_s *mailslots = kmalloc(sizeof(struct mailslot_vector_s), GFP_KERNEL);
    if (mailslots == NULL)
    {
        printk("kmalloc failed while allocating the mailslot vector\n");
        goto end;
    }
    mailslots->number_of_instances = 0;
    mailslots->next = 0;
    memset(mailslots->mailslot_instances, 0, MINORS);

end:
    return mailslots;
}

void insert_new_mailslot(struct mailslot_vector_s *mailslot_vector, int device_instance, struct mailslot_s *mailslot)
{
    mailslot_vector->mailslot_instances[device_instance] = mailslot;
    mailslot_vector->number_of_instances += 1;
    mailslot_vector->next += 1;
}

void remove_mailslot_instance(mailslot_vector_t mailslot_vector, int device_instance)
{
    mailslot_vector->mailslot_instances[device_instance] = NULL;
    mailslot_vector->next = device_instance;
    mailslot_vector->number_of_instances -= 1;
}

mailslot_t get_mailslot(struct mailslot_vector_s *mailslots, int device_instance)
{
    return mailslots->mailslot_instances[device_instance];
}