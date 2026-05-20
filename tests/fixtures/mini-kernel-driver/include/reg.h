/* Shared registration structures for the mini-kernel-driver test fixture */

struct file_operations {
    struct module *owner;
    loff_t (*llseek)(struct file *, loff_t, int);
    ssize_t (*read)(struct file *, char __user *, size_t, loff_t *);
    ssize_t (*write)(struct file *, const char __user *, size_t, loff_t *);
    long (*unlocked_ioctl)(struct file *, unsigned int, unsigned long);
    int (*open)(struct inode *, struct file *);
    int (*release)(struct inode *, struct file *);
};

struct i2c_driver {
    int (*probe)(struct i2c_client *, const struct i2c_device_id *);
    int (*remove)(struct i2c_client *);
    struct device_driver driver;
};

struct platform_driver {
    int (*probe)(struct platform_device *);
    int (*remove)(struct platform_device *);
    struct device_driver driver;
};
