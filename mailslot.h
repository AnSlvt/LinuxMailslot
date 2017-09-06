#include "mailslot_vector.h"

#ifndef MAILSLOT_H
#define MAILSLOT_H

struct mailslot_s
{
    struct msg_obj_s **mails;
    int next_to_read;           /* mails[next_to_read % current_max_msgs] is the full slot */
    int next_to_insert;         /* mails[next_to_insert % current_max_msgs] is the first empty slot */

    struct semaphore *full;     /* keep track of the number of full spots */
    struct semaphore *empty;    /* keep track of the number of empty spots */
    spinlock_t *mailslot_sync;  /* enforce mutual exclusion to shared data */

    int current_max_msgs;
    int current_msg_size;
    int mails_in;
};

typedef struct mailslot_s * mailslot_t;

void create_new_mailslot(mailslot_vector_t mailslots, int device_instance);

void insert_new_msg(mailslot_t mailslot, struct msg_obj_s *msg);

int read_msg(mailslot_t mailslot, char *buff, int len);

//struct msg_obj_s *get_mail(mailslot_t mailslot, int position);

void free_mailslot(mailslot_t mailslot);
/*
int get_mails_in(mailslot_t mailslot);

int is_length_compatible(mailslot_t mailslot, int len);

int there_is_space(mailslot_t mailslot);

int is_empty(mailslot_t mailslot);*/

#endif