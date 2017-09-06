#ifndef MAILSLOT_VECTOR_H
#define MAILSLOT_VECTOR_H

#include "macro.h"

struct mailslot_vector_s
{
    struct mailslot_s *mailslot_instances[MINORS];
    int number_of_instances;
    int next;
};

typedef struct mailslot_vector_s * mailslot_vector_t;

struct mailslot_vector_s * create_new_mailslot_vector(void);
bool_t insert_new_mailslot(mailslot_vector_t mailslot_vector, int device_instance, struct mailslot_s *mailslot);
struct mailslot_s * get_mailslot(mailslot_vector_t mailslots, int device_instance);
void remove_mailslot_instance(mailslot_vector_t mailslot_vector, int device_instance);

#endif