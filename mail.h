#ifndef MAIL_H
#define MAIL_H

struct msg_obj_s
{
    struct mailslot_s *mailslot;
    char *msg;
    int msg_len;
};

typedef struct msg_obj_s * msg_obj_t;

msg_obj_t create_new_msg(struct mailslot_s *mailslot, const char *msg, int len, int *error);
void free_msg(struct msg_obj_s *msg);
char *get_msg(msg_obj_t mail);
int get_msg_len(msg_obj_t mail);

#endif