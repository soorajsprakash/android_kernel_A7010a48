/*
 * Copyright � 2017, DarkBlood <gabro2003@gmail.com>
 *
 * Post-processing controller for MTK
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Please preserve this licence and driver name if you implement this
 * anywhere else.
 *
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include "pp_control.h"

#define pp_control_version 1
#define pp_control_subversion 6

static void update_pp(struct pp_data *pp_data)
{
	int i, gammutR, gammutG, gammutB;
	unsigned char h_series[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int u4Temp = 0;
	int index = 0;
	DISP_GAMMA_LUT_T *gamma;

	if (pp_data->enable) {
		_color_reg_mask(NULL, DISP_COLOR_CFG_MAIN + offset, (0 << 7), 0x00000080);
		_color_reg_set(NULL, DISP_COLOR_START + offset, 0x00000001);
		_color_reg_set(NULL, DISP_COLOR_CM1_EN + offset, 0x01);
		_color_reg_set(NULL, DISP_COLOR_CM2_EN + offset, 0x11);
	} else {
		_color_reg_set(NULL, DISP_COLOR_CM1_EN + offset, 0);
		_color_reg_set(NULL, DISP_COLOR_CM2_EN + offset, 0);
		_color_reg_mask(NULL, DISP_COLOR_CFG_MAIN + offset, (1 << 7), 0x00000080);
		_color_reg_set(NULL, DISP_COLOR_START + offset, 0x00000003);
	}


	if (pp_data->red < pp_data->minimum) {
	pp_data->red = pp_data->minimum;
	}
	if (pp_data->green < pp_data->minimum) {
	pp_data->green = pp_data->minimum;
	}
	if (pp_data->blue < pp_data->minimum) {
	pp_data->blue = pp_data->minimum;
	}

	gamma = kzalloc(sizeof(DISP_GAMMA_LUT_T), GFP_KERNEL);
	gamma->hw_id = 0;

	if (pp_data->enable) {
	for (i = 0; i < 512; i++) {
		gammutR = i * pp_data->red / PROGRESSION_SCALE;
		gammutG = i * pp_data->green / PROGRESSION_SCALE;
		gammutB = i * pp_data->blue / PROGRESSION_SCALE;

		gamma->lut[i] = GAMMA_ENTRY(gammutR, gammutG, gammutB);
	} } else {
	for (i = 0; i < 512; i++) {
		gammutR = i * 2000 / PROGRESSION_SCALE;
		gammutG = i * 2000 / PROGRESSION_SCALE;
		gammutB = i * 2000 / PROGRESSION_SCALE;

		gamma->lut[i] = GAMMA_ENTRY(gammutR, gammutG, gammutB);
	}
	}

	primary_display_user_cmd(DISP_IOCTL_SET_GAMMALUT, (unsigned long)gamma);
	kfree(gamma);
	_color_reg_set(NULL, DISP_COLOR_G_PIC_ADJ_MAIN_2,
	pp_data->sat);
	_color_reg_set(NULL, DISP_COLOR_G_PIC_ADJ_MAIN_1,
	(pp_data->brightness << 16) | pp_data->cont);
	for (index = 0; index < 20; index++) {
		h_series[index] = pp_data->hue;
	}

	for (index = 0; index < 5; index++) {
		u4Temp = (h_series[4 * index]) +
		    (h_series[4 * index + 1] << 8) +
		    (h_series[4 * index + 2] << 16) + (h_series[4 * index + 3] << 24);
		_color_reg_set(NULL, DISP_COLOR_LOCAL_HUE_CD_0 + offset + 4 * index, u4Temp);
	}
	if (pp_data->enable) {
	color_trigger_refresh(DISP_MODULE_COLOR0);
	}

}

static ssize_t min_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int min, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
	return -EINVAL;

	ret = sscanf(buf, "%d", &min);
	if ((!ret) || (min < 1 || min > 2000))
		return -EINVAL;

	pp_data->minimum = min;
	update_pp(pp_data);
	return count;
}

static ssize_t min_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->minimum);
}

static ssize_t green_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int green, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &green);
	if ((!ret) || (green > 2000))
		return -EINVAL;

	pp_data->green = green;
	update_pp(pp_data);
	return count;
}

static ssize_t green_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->green);
}

static ssize_t blue_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int blue, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &blue);
	if ((!ret) || (blue > 2000))
		return -EINVAL;

	pp_data->blue = blue;
	update_pp(pp_data);
	return count;
}

static ssize_t blue_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->blue);
}

static ssize_t red_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int red, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	ret = sscanf(buf, "%d", &red);
	if ((!ret) || (red > 2000))
		return -EINVAL;

	pp_data->red = red;
	update_pp(pp_data);
	return count;
}

static ssize_t red_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->red);
}

static ssize_t enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int enable, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &enable);
	if ((!ret) || (enable != 0 && enable != 1) ||
		(pp_data->enable == enable))
		return -EINVAL;

	pp_data->enable = enable;
	update_pp(pp_data);
	return count;
}

static ssize_t enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->enable);
}

static ssize_t sat_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int sat, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	ret = sscanf(buf, "%d", &sat);

	pp_data->sat = sat;
	update_pp(pp_data);
	return count;
}

static ssize_t sat_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	pp_data->sat = DISP_REG_GET(DISP_COLOR_G_PIC_ADJ_MAIN_2);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->sat);
}

static ssize_t cont_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int cont, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd()) {
	return -EINVAL;
	}

	ret = sscanf(buf, "%d", &cont);

	pp_data->cont = cont;
	update_pp(pp_data);
	return count;
}

static ssize_t cont_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->cont);
}

static ssize_t brig_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int brig, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &brig);

	pp_data->brightness = brig;
	update_pp(pp_data);
	return count;
}

static ssize_t brig_show(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->brightness);
}

static ssize_t hue_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->hue);
}

static ssize_t hue_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count) {
	int hue, ret;
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &hue);

	pp_data->hue = hue;
	update_pp(pp_data);
	return count;
}

static ssize_t invert_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int invert, ret;

	extern void lcm_set_inversemode(bool enable);
	struct pp_data *pp_data = dev_get_drvdata(dev);

	if (primary_display_is_sleepd())
		return -EINVAL;

	ret = sscanf(buf, "%d", &invert);
	if (!ret || (invert != 0 && invert != 1) ||
	    pp_data->invert == invert)
		return -EINVAL;

	pp_data->invert = invert;
	lcm_set_inversemode(invert);
	return count;
}

static ssize_t invert_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct pp_data *pp_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pp_data->invert);
}

static ssize_t version_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "v%d_%d\n",
			 pp_control_version, pp_control_subversion);
}

static DEVICE_ATTR(hue, S_IWUSR | S_IRUGO, hue_show, hue_store);
static DEVICE_ATTR(sat, S_IWUSR | S_IRUGO, sat_show, sat_store);
static DEVICE_ATTR(green, S_IWUSR | S_IRUGO, green_show, green_store);
static DEVICE_ATTR(blue, S_IWUSR | S_IRUGO, blue_show, blue_store);
static DEVICE_ATTR(red, S_IWUSR | S_IRUGO, red_show, red_store);
static DEVICE_ATTR(brightness, S_IWUSR | S_IRUGO, brig_show, brig_store);
static DEVICE_ATTR(cont, S_IWUSR | S_IRUGO, cont_show, cont_store);
static DEVICE_ATTR(min, S_IWUSR | S_IRUGO, min_show, min_store);
static DEVICE_ATTR(version, S_IWUSR | S_IRUGO, version_show, NULL);
static DEVICE_ATTR(enable, S_IWUSR | S_IRUGO, enable_show, enable_store);
static DEVICE_ATTR(invert, S_IWUSR | S_IRUGO, invert_show, invert_store);

static int pp_control_probe(struct platform_device *pdev)
{
	int ret;
	struct pp_data *pp_data;

	pp_data = devm_kzalloc(&pdev->dev, sizeof(*pp_data), GFP_KERNEL);
	if (!pp_data) {
		pr_err("%s: failed to allocate memory for pp_data\n",
			__func__);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, pp_data);

	pp_data->enable = 1;
	pp_data->red = MAX_LUT_SCALE;
	pp_data->green = MAX_LUT_SCALE;
	pp_data->blue = MAX_LUT_SCALE;
	pp_data->minimum = 1;
	pp_data->sat = DISP_REG_GET(DISP_COLOR_G_PIC_ADJ_MAIN_2);
	pp_data->cont = 128;
	pp_data->brightness = 1024;

	pp_data->hue = 128;
	pp_data->invert = 0;

	ret = device_create_file(&pdev->dev, &dev_attr_min);
	ret |= device_create_file(&pdev->dev, &dev_attr_cont);
	ret |= device_create_file(&pdev->dev, &dev_attr_enable);
	ret |= device_create_file(&pdev->dev, &dev_attr_sat);
	ret |= device_create_file(&pdev->dev, &dev_attr_version);
	ret |= device_create_file(&pdev->dev, &dev_attr_brightness);
	ret |= device_create_file(&pdev->dev, &dev_attr_hue);
	ret |= device_create_file(&pdev->dev, &dev_attr_blue);
	ret |= device_create_file(&pdev->dev, &dev_attr_green);
	ret |= device_create_file(&pdev->dev, &dev_attr_red);
	ret |= device_create_file(&pdev->dev, &dev_attr_invert);

	if (ret) {
		pr_err("%s: unable to create sysfs entries\n", __func__);
		return ret;
	}

	return 0;
}

static int pp_control_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_min);
	device_remove_file(&pdev->dev, &dev_attr_enable);
	device_remove_file(&pdev->dev, &dev_attr_sat);
	device_remove_file(&pdev->dev, &dev_attr_cont);
	device_remove_file(&pdev->dev, &dev_attr_version);
	device_remove_file(&pdev->dev, &dev_attr_brightness);
	device_remove_file(&pdev->dev, &dev_attr_hue);
	device_remove_file(&pdev->dev, &dev_attr_blue);
	device_remove_file(&pdev->dev, &dev_attr_green);
	device_remove_file(&pdev->dev, &dev_attr_red);
	device_remove_file(&pdev->dev, &dev_attr_invert);

	return 0;
}

static struct platform_driver pp_control_driver = {
	.probe = pp_control_probe,
	.remove = pp_control_remove,
	.driver = {
		.name = "pp_control",
	},
};

static struct platform_device pp_control_device = {
	.name = "pp_control",
};

static int __init pp_control_init(void)
{
	if (platform_driver_register(&pp_control_driver))
		return -ENODEV;

	if (platform_device_register(&pp_control_device))
		return -ENODEV;

	pr_info("%s: registered\n", __func__);

	return 0;
}

static void __exit pp_control_exit(void)
{
	platform_driver_unregister(&pp_control_driver);
	platform_device_unregister(&pp_control_device);
}

module_init(pp_control_init);
module_exit(pp_control_exit);
MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("DarkBlood <gabro2003@gmail.com>");
MODULE_DESCRIPTION("Post processing control driver");
