#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x39d022bc, "module_layout" },
	{ 0x70060a39, "platform_driver_unregister" },
	{ 0x992c330a, "__platform_driver_register" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x4d2887ca, "class_destroy" },
	{ 0x752ff22c, "device_destroy" },
	{ 0x1dc0fa1, "cdev_del" },
	{ 0x76e8f9e8, "_dev_info" },
	{ 0x72043dea, "cdev_add" },
	{ 0x26e1448b, "device_create" },
	{ 0x64c8e5d3, "cdev_init" },
	{ 0x5cfd04d0, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x2aea69ff, "devm_ioremap_resource" },
	{ 0x81270d16, "platform_get_resource" },
	{ 0x4277a3b0, "_dev_err" },
	{ 0x64d65571, "devm_kmalloc" },
	{ 0x5f754e5a, "memset" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xe767086, "remap_pfn_range" },
	{ 0xcea47a2d, "phys_mem_access_prot" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x581cde4e, "up" },
	{ 0xc5850110, "printk" },
	{ 0xca5a7528, "down_interruptible" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Ccapstone,pwm_driver");
MODULE_ALIAS("of:N*T*Ccapstone,pwm_driverC*");
