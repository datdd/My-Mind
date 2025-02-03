#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"
#define IOCTL_MAGIC 'x' // Magic number for ioctl commands
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, int) // Set an integer value
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 2, int) // Get the integer value

static dev_t dev_num;
static struct cdev my_cdev;
static int device_value = 0; // A variable to store the value set by ioctl

static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: Device closed\n");
    return 0;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTL_SET_VALUE:
            if (copy_from_user(&device_value, (int __user *)arg, sizeof(device_value)))
                return -EFAULT;
            printk(KERN_INFO "mychardev: IOCTL SET_VALUE = %d\n", device_value);
            break;

        case IOCTL_GET_VALUE:
            if (copy_to_user((int __user *)arg, &device_value, sizeof(device_value)))
                return -EFAULT;
            printk(KERN_INFO "mychardev: IOCTL GET_VALUE = %d\n", device_value);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .unlocked_ioctl = my_ioctl,
};

static int __init my_init(void) {
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ERR "Failed to allocate device number\n");
        return -1;
    }

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "mychardev: Registered with major %d, minor %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit my_exit(void) {
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "mychardev: Unregistered\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
