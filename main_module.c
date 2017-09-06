#define EXPORT_SYMTAB
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/version.h>

#include <asm/ioctl.h>

#include "macro.h"

#include "mailslot_vector.h"
#include "mailslot.h"
#include "mail.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Salvati");

/*#define IOCTL_MAGIC 0xF7
#define SEQUENCE_CMD_BLOCKING 0x0001
#define SEQUENCE_CMD_NONBLOCKIN 0x0010
#define NONE_DIRECTION 0x000000 | _IOC_NONE*/

static int Major;

mailslot_vector_t mailslots;

static int open_mailslot_instance(struct inode *inode, struct file *file)
{
    int device_instance = MINOR(inode->i_rdev);

    printk("%s: somebody called an open on the mailslot istance [major,minor] number [%d,%d]\n", 
        DEVICE_NAME, 
        MAJOR(inode->i_rdev), 
        device_instance);

    create_new_mailslot(mailslots, device_instance);

    return 0;
}

static int mailslot_instance_release(struct inode *inode, struct file *file)
{
    int device_instance = MINOR(inode->i_rdev);
    mailslot_t to_release;

    printk("%s: somebody called a release on the mailslot istance [major,minor] number [%d,%d]\n", 
        DEVICE_NAME, 
        MAJOR(inode->i_rdev), 
        device_instance);

    if ((to_release = get_mailslot(mailslots, device_instance)) != NULL)
    {
        free_mailslot(to_release);
        remove_mailslot_instance(mailslots, device_instance);
    }

   return 0;
}

static ssize_t write_on_mailslot(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    msg_obj_t mail;
    int device_instance = MINOR(filp->f_inode->i_rdev);
    mailslot_t mailslot = get_mailslot(mailslots, device_instance);

    printk("%s: somebody called a write on the mailslot istance [majors,minor] number [%d,%d]\n", 
        DEVICE_NAME, 
        MAJOR(filp->f_inode->i_rdev), 
        device_instance);

    //if (!is_length_compatible(mailslot, len) || !there_is_space(mailslot)) return -1;

    mail = create_new_msg(mailslot, buff, len);
    if (mail == NULL)
    {
        printk("%s: the length of the message is not compatible with the mailslot\n", DEVICE_NAME);
        return 0;
    }
    insert_new_msg(mailslot, mail);

    return len;
}

static ssize_t read_from_mailslot(struct file *filp, char *buff, size_t len, loff_t *off)
{
    int device_instance = MINOR(filp->f_inode->i_rdev);
    mailslot_t mailslot = get_mailslot(mailslots, device_instance);
    int ret;

    printk("%s: somebody called a read on the mailslot istance [majors,minor] number [%d,%d] and wants to read %lu bytes\n", 
        DEVICE_NAME, 
        MAJOR(filp->f_inode->i_rdev), 
        device_instance, len);

    ret = read_msg(mailslot, buff, len);

    return ret;
}

/*static long ioctl_mailslot(/*struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    unsigned int cmd_switch_flag = IOCTL_MAGIC | SEQUENCE_CMD_BLOCKING | NONE_DIRECTION;
    printk("%s: switch flag %d\n", DEVICE_NAME, cmd_switch_flag);

    return 0;
}*/

static struct file_operations fops = {
    .open    = open_mailslot_instance,
    .write   = write_on_mailslot,
    .read    = read_from_mailslot,
    .release = mailslot_instance_release//,
    //.unlocked_ioctl   = ioctl_mailslot
};






int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0)
    {
        printk("Registering mail_slot device failed\n");
        return Major;
    }

    mailslots = create_new_mailslot_vector();
    if (mailslots == NULL) return -1;

    printk(KERN_INFO "mail_slot device registered, it is assigned major number %d\n", Major);

    return 0;
}

void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO "mailslot device unregistered, it was assigned major number %d\n", Major);
    kfree(mailslots);
}