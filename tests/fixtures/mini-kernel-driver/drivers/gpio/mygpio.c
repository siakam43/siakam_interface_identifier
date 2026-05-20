/* drivers/gpio/mygpio.c — GPIO controller driver with IRQ, timer, sysfs */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>
#include <linux/device.h>

static int gpio_irq;
static struct timer_list gpio_timer;
static struct device *gpio_dev;

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=sysfs_handler
 * registration: sysfs_create_file at drivers/gpio/mygpio.c:62
 * external_module: Linux sysfs / userspace
 * data_flow: userspace → read() on sysfs attribute → sysfs_state_show(struct device *, struct device_attribute *, char *)
 *
 * Reason: Registered as .show callback via sysfs_create_file (or DEVICE_ATTR).
 * Reads from userspace via the sysfs virtual filesystem. Provides device state
 * to userspace readers.
 */
ssize_t sysfs_state_show(struct device *dev, struct device_attribute *attr,
                          char *buf)
{
    int state = gpio_get_value(42);
    return sprintf(buf, "%d\n", state);
}
static DEVICE_ATTR(state, 0444, sysfs_state_show, NULL);

/*
 * [SIAKAM_EXPECT] confidence=medium
 * interface_type=irq_handler
 * registration: request_irq at drivers/gpio/mygpio.c:89
 * external_module: Linux IRQ subsystem / external GPIO hardware
 * data_flow: external GPIO pin edge → interrupt controller → gpio_irq_handler(int, void *)
 *
 * Reason: Registered via request_irq for an external GPIO pin interrupt.
 * IRQ source is off-chip hardware (external GPIO expander). Medium confidence
 * because the gpio_is_external() check confirms off-chip source, but exact
 * hardware boundary depends on board design.
 */
irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    int val = gpio_get_value(gpio_irq);
    pr_info("GPIO IRQ: pin=%d val=%d\n", gpio_irq, val);
    return IRQ_HANDLED;
}

/*
 * [SIAKAM_EXPECT] exclude
 * exclusion_reason=internal_callback_only
 *
 * Reason: Registered via timer_setup — purely internal kernel scheduling
 * mechanism. No external data source or caller. Timer callbacks are
 * kernel-internal bookkeeping.
 */
void gpio_timer_fn(struct timer_list *t)
{
    mod_timer(&gpio_timer, jiffies + HZ);
    pr_debug("gpio timer tick\n");
}

static int __init mygpio_init(void)
{
    int ret;

    gpio_irq = 42;
    ret = request_irq(gpio_irq, gpio_irq_handler,
                      IRQF_TRIGGER_RISING, "mygpio", NULL);

    timer_setup(&gpio_timer, gpio_timer_fn, 0);
    mod_timer(&gpio_timer, jiffies + HZ);

    ret = sysfs_create_file(&gpio_dev->kobj, &dev_attr_state.attr);

    return ret;
}
module_init(mygpio_init);

MODULE_LICENSE("GPL");
