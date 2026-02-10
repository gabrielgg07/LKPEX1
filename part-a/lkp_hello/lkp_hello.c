/*
 * lkp_hello.c - Hello LKP Module (Exercise 1, Part A.1)
 *
 * A simple kernel module that prints a greeting on load/unload
 * and accepts module parameters.
 */
#define pr_fmt(fmt) "lkp: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gabriel Gonzalez");
MODULE_DESCRIPTION("LKP: Hello Module");

static char *name = "LKP";
module_param(name, charp, 0644);
MODULE_PARM_DESC(name, "Name to greet");

/* TODO: Add count parameter (int, default 1, permissions 0644) */

static int count = 1; //Standard int creation static so it belongs to this module
module_param(count, int, 0644); // 0644 Owner can read/write others can only read
MODULE_PARM_DESC(count, "Number of times to greet, default 1");

static int __init lkp_hello_init(void)
{
	/* TODO: Print greeting 'count' times using pr_info() */
	for(int i = 0; i < count; i++){
		//Basically just a wrapper around printk for cleaner syntax. 
	    pr_info("Hello, %s!\n", name);
	}
	return 0;
}

static void __exit lkp_hello_exit(void)
{
	pr_info("Goodbye, %s!\n", name);
}

module_init(lkp_hello_init);
module_exit(lkp_hello_exit);
