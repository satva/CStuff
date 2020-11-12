/*
 * Linear Technology LTC2991 I2C Voltage/Temperature Monitor
 *
 * Copyright (c) 2014 Rockwell Collins
 *
 * Based on: Linear Technology LTC2945 I2C Power Monitor
 * Copyright (c) 2014 Guenter Roeck
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/jiffies.h>

/* chip registers */
#define LTC2991_STATUS_L		0x00
#define LTC2991_STATUS_H		0x01
#define LTC2991_CONTROL_V_L		0x06
#define LTC2991_CONTROL_V_H		0x07
#define LTC2991_CONTROL_PWM_L		0x08
#define LTC2991_CONTROL_PWM_H		0x09
#define LTC2991_V1_H			0x0a
#define LTC2991_V1_L			0x0b
#define LTC2991_V2_H			0x0c
#define LTC2991_V2_L			0x0d
#define LTC2991_V3_H			0x0e
#define LTC2991_V3_L			0x0f
#define LTC2991_V4_H			0x10
#define LTC2991_V4_L			0x11
#define LTC2991_V5_H			0x12
#define LTC2991_V5_L			0x13
#define LTC2991_V6_H			0x14
#define LTC2991_V6_L			0x15
#define LTC2991_V7_H			0x16
#define LTC2991_V7_L			0x17
#define LTC2991_V8_H			0x18
#define LTC2991_V8_L			0x19
#define LTC2991_ITMP_H			0x1a
#define LTC2991_ITMP_L			0x1b
#define LTC2991_VCC_H			0x1c
#define LTC2991_VCC_L			0x1d

/* Control register bits */

#define CONTROL_EN_VCC_ITMP		(1 << 3)
#define CONTROL_EN_CHAN7_8		(1 << 7)
#define CONTROL_EN_CHAN7_8_V		(0)
#define CONTROL_EN_CHAN5_6		(1 << 6)
#define CONTROL_EN_CHAN5_6_V		(0)
#define CONTROL_EN_CHAN3_4		(1 << 5)
#define CONTROL_EN_CHAN3_4_V		(0)
#define CONTROL_EN_CHAN1_2		(1 << 4)
#define CONTROL_EN_CHAN1_2_V		(0)
#define LTC2991_MEASURE_TEMP_MASK	0x1FFF
#define LTC2991_MEASURE_VIN_MASK	0x3FFF
#define LTC2991_MEASURE_VIN_SIGN_MASK	0x4000


#define KELVIN_TO_MILLIDEGREES_CELSIUS	2731500
#define LTC2991_T_ADC	625  /*Reference: LTC2991 Datasheet Pg.no:13*/
#define LTC2991_V_ADC	30518 /*Reference: LTC2991 Datasheet Pg.no:14*/


struct ltc2991_data {
	struct device *hwmon_dev;
};

/* Return the output in micro volts from an input hex value of a register */
static long calc_voltage(u32 buf) {
	long val=0;

	if(buf & LTC2991_MEASURE_VIN_SIGN_MASK)
		buf *= -1;
	val = (buf & LTC2991_MEASURE_VIN_MASK) * LTC2991_V_ADC;
	val/=100;

	return val;
}

/* Return the value from the given register in mV */
static long long ltc2991_reg_to_val(struct device *dev, u8 reg){
	struct i2c_client *client = to_i2c_client(dev);
	u32 buf;
	long val=0;

	switch (reg) {
		/*Voltage in micro volts
		Temperature in milli degree celsius.*/
	case LTC2991_V1_H:
	case LTC2991_V2_H:
	case LTC2991_V3_H:
	case LTC2991_V4_H:
	case LTC2991_V5_H:
	case LTC2991_V6_H:
	case LTC2991_V7_H:
	case LTC2991_V8_H:
		buf = i2c_smbus_read_word_swapped(client,reg);
		val = calc_voltage(buf);
		break;
	case LTC2991_VCC_H:
		buf = i2c_smbus_read_word_swapped(client,LTC2991_VCC_H);
		val = calc_voltage(buf);
		val+= 2500000; /*Vcc = Result+2.5Volts as per datasheet*/
		break;
	case LTC2991_ITMP_H:
		buf = i2c_smbus_read_word_swapped(client,LTC2991_ITMP_H);
		val = (buf & LTC2991_MEASURE_TEMP_MASK) * LTC2991_T_ADC;
		dev_dbg(&dev,"Temp Internal in K:%ld\n",val);
		val-= KELVIN_TO_MILLIDEGREES_CELSIUS; /*Converting Kelvin to Celsius*/
		val/=10; /*Final Conversion to milli degress Celsius*/
		break;
	default:
		return -EINVAL;
	}

	return val;
}

static ssize_t ltc2991_show_value(struct device *dev,
				  struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	long value;

	value = ltc2991_reg_to_val(dev, attr->index);

	return snprintf(buf, PAGE_SIZE, "%ld\n", value);
}

/*Information directly derived from register values*/
static SENSOR_DEVICE_ATTR(in1_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V1_H);
static SENSOR_DEVICE_ATTR(in2_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V2_H);
static SENSOR_DEVICE_ATTR(in3_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V3_H);
static SENSOR_DEVICE_ATTR(in4_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V4_H);
static SENSOR_DEVICE_ATTR(in5_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V5_H);
static SENSOR_DEVICE_ATTR(in6_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V6_H);
static SENSOR_DEVICE_ATTR(in7_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V7_H);
static SENSOR_DEVICE_ATTR(in8_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_V8_H);

/* Internal VCC and temperature probes */
static SENSOR_DEVICE_ATTR(in0_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_VCC_H);
static SENSOR_DEVICE_ATTR(temp0_input, S_IRUGO, ltc2991_show_value, NULL,
			  LTC2991_ITMP_H);


static struct attribute *ltc2991_attributes[] = {
	&sensor_dev_attr_in0_input.dev_attr.attr,
	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_in3_input.dev_attr.attr,
	&sensor_dev_attr_in4_input.dev_attr.attr,
	&sensor_dev_attr_in5_input.dev_attr.attr,
	&sensor_dev_attr_in6_input.dev_attr.attr,
	&sensor_dev_attr_in7_input.dev_attr.attr,
	&sensor_dev_attr_in8_input.dev_attr.attr,
	&sensor_dev_attr_temp0_input.dev_attr.attr,
	NULL,
};

static const struct attribute_group ltc2991_group = {
	.attrs = ltc2991_attributes,
};

static int ltc2991_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = client->adapter;
	struct ltc2991_data *data;
	int ret;

	dev_dbg(&dev,"i2c_client:name-%s\n",client->name);
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA))
		return -ENODEV;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);

	/*Device Configuration*/
	ret = sysfs_create_group(&client->dev.kobj, &ltc2991_group);
	if (ret)
		goto out_err_sysfs;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		ret = PTR_ERR(data->hwmon_dev);
		goto out_err_hwmon;
	}

	/*Set Control Registers.This enables the chip to start sampling.
	Configured for Kelvin,Noise filter.*/
	i2c_smbus_write_byte_data(client,LTC2991_STATUS_H,0xf8);
	i2c_smbus_write_byte_data(client,LTC2991_CONTROL_V_L,0x00);
	i2c_smbus_write_byte_data(client,LTC2991_CONTROL_V_H,0x00);
	i2c_smbus_write_byte_data(client,LTC2991_CONTROL_PWM_L,0x3c);

	return 0;

out_err_hwmon:
	sysfs_remove_group(&client->dev.kobj, &ltc2991_group);
	return ret;

out_err_sysfs:
	devm_kfree(dev,data);
	return ret;
}

static int ltc2991_remove(struct i2c_client *client)
	{
		struct ltc2991_data *data = i2c_get_clientdata(client);
		hwmon_device_unregister(data->hwmon_dev);
		sysfs_remove_group(&client->dev.kobj, &ltc2991_group);

		return 0;
	}



static const struct of_device_id ltc2991_of_match_table[] = {
	{.compatible = "ltc,ltc2991", },
	{},
};


static const struct i2c_device_id ltc2991_id[] = {
	{"ltc2991", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ltc2991_id);

static struct i2c_driver ltc2991_driver = {
	.driver = {
		.name = "ltc2991",
		.of_match_table = ltc2991_of_match_table,
		},
	.probe = ltc2991_probe,
	.remove = ltc2991_remove,
	.id_table = ltc2991_id,
};

module_i2c_driver(ltc2991_driver);

MODULE_AUTHOR("Matt Weber <Matthew.Weber@rockwellcollins.com>");
MODULE_DESCRIPTION("LTC2991 driver");
MODULE_LICENSE("GPL");
