#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define MY_MAJOR 200  // Set a specific major number

static char buffer[256];
static struct cdev my_cdev;
static dev_t dev_num;

static ssize_t my_read(struct file *filp, char __user *user_buf, size_t len, loff_t *offset) {
    return simple_read_from_buffer(user_buf, len, offset, buffer, sizeof(buffer));
}

static ssize_t my_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *offset) {
    return simple_write_to_buffer(buffer, sizeof(buffer), offset, user_buf, len);
}

static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .read    = my_read,
    .write   = my_write,
    .open    = my_open,
    .release = my_release
};

// static int __init my_init(void) {
//     if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
//         printk(KERN_ERR "Failed to allocate device number\n");
//         return -1;
//     }

//     cdev_init(&my_cdev, &fops);
//     if (cdev_add(&my_cdev, dev_num, 1) < 0) {
//         unregister_chrdev_region(dev_num, 1);
//         return -1;
//     }

//     printk(KERN_INFO "mychardev registered with major %d, minor %d\n", MAJOR(dev_num), MINOR(dev_num));
//     return 0;
// }

static int __init my_init(void) {
    dev_t dev_num = MKDEV(MY_MAJOR, 0);

    if (register_chrdev_region(dev_num, 1, "mychardev") < 0) {
        printk(KERN_ERR "Failed to register device\n");
        return -1;
    }

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "mychardev registered with major %d, minor %d\n", MY_MAJOR, 0);
    return 0;
}

static void __exit my_exit(void) {
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "mychardev unregistered\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
