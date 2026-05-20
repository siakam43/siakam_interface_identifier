/* drivers/char/mychar.c — character device driver */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "../include/reg.h"

#define DEVICE_NAME "mychar"

static int major;

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=ioctl_handler
 * registration: file_operations.unlocked_ioctl at drivers/char/mychar.c:56
 * external_module: Linux VFS / userspace
 * data_flow: userspace → ioctl(fd, cmd, arg) → mychar_ioctl(struct file *, unsigned int, unsigned long)
 *
 * Reason: Registered in struct file_operations.unlocked_ioctl. The arg parameter
 * carries a userspace pointer (unsigned long cast from void __user *), making this
 * an untrusted data entry point from userspace.
 */
/* [SIAKAM_EXPECT] confidence=high */
long mychar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int __user *user_ptr = (int __user *)arg;

    switch (cmd) {
    case 0x01:
        return put_user(42, user_ptr);
    default:
        return -ENOTTY;
    }
}

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=file_operations_handler
 * registration: file_operations.read at drivers/char/mychar.c:73
 * external_module: Linux VFS / userspace
 * data_flow: userspace → read(fd, buf, len) → mychar_read(struct file *, char __user *, size_t, loff_t *)
 */
ssize_t mychar_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    const char *data = "hello";
    if (*off >= 5)
        return 0;
    if (copy_to_user(buf, data + *off, 1))
        return -EFAULT;
    (*off)++;
    return 1;
}

/* [SIAKAM_EXPECT] confidence=high */
int mychar_open(struct inode *inode, struct file *filp)
{
    return 0;
}

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=exported_symbol
 * registration: EXPORT_SYMBOL at drivers/char/mychar.c:99
 * external_module: external kernel modules
 * data_flow: external kernel module → mydrv_get_version(void) → const char *
 */
const char *mydrv_get_version(void)
{
    return "mychar v1.0";
}
EXPORT_SYMBOL(mydrv_get_version);

/* Registration table */
static struct file_operations mychar_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = mychar_ioctl,
    .read           = mychar_read,
    .open           = mychar_open,
};

static int __init mychar_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &mychar_fops);
    return major >= 0 ? 0 : major;
}
module_init(mychar_init);

MODULE_LICENSE("GPL");
