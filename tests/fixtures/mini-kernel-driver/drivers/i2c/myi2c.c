/* drivers/i2c/myi2c.c — I2C device driver */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include "../include/reg.h"

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=i2c_driver_probe
 * registration: i2c_driver.probe at drivers/i2c/myi2c.c:50
 * external_module: Linux I2C subsystem
 * data_flow: I2C bus enumeration → i2c core → myi2c_probe(struct i2c_client *, const struct i2c_device_id *)
 *
 * Reason: Registered as .probe callback in struct i2c_driver. Called by the
 * I2C core when a matching device is found on the bus. Receives i2c_client
 * pointer carrying device tree / ACPI configuration data from firmware.
 */
int myi2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    struct device *dev = &client->dev;

    ret = device_property_read_u32(dev, "my,reg-addr", &addr);
    if (ret)
        return ret;

    dev_info(dev, "myi2c probed at 0x%02x\n", client->addr);
    return 0;
}

/*
 * [SIAKAM_EXPECT] confidence=high
 * interface_type=i2c_driver_remove
 * registration: i2c_driver.remove at drivers/i2c/myi2c.c:70
 * external_module: Linux I2C subsystem
 * data_flow: device unbind/hotplug → i2c core → myi2c_remove(struct i2c_client *)
 */
int myi2c_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "myi2c removed\n");
    return 0;
}

static const struct of_device_id myi2c_of_match[] = {
    { .compatible = "vendor,myi2c" },
    { }
};
MODULE_DEVICE_TABLE(of, myi2c_of_match);

static struct i2c_driver myi2c_driver = {
    .probe  = myi2c_probe,
    .remove = myi2c_remove,
    .driver = {
        .name = "myi2c",
        .of_match_table = myi2c_of_match,
    },
};
module_i2c_driver(myi2c_driver);

MODULE_LICENSE("GPL");
