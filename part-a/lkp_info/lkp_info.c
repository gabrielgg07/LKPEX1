/*
 * lkp_info.c - /proc Interface Module (Exercise 1, Part A.2)
 *
 * Creates /proc/lkp_info that shows module uptime and access count
 * using the seq_file interface.
 */
#define pr_fmt(fmt) "lkp: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/atomic.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gabriel Gonzalez");
MODULE_DESCRIPTION("LKP: Info Module with /proc Interface");

static struct proc_dir_entry *proc_entry;
static unsigned long load_jiffies;

/*Declare an atomic_t counter for tracking access count */

static atomic_t counter = ATOMIC_INIT(0);  // Create atomic counter belonging to this module

static int lkp_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "LKP Info Module\n");
	seq_printf(m, "===============\n");


	/**  Show load time (jiffies at load) */
	seq_printf(m, "Load time (jiffies): ");
	seq_printf(m,"%lu\n",load_jiffies); //The lu says that the arguemtn is an unsigned long.

	//Get current jiffies
	unsigned long now = jiffies;
	seq_printf(m, "Current jiffies: ");
	seq_printf(m,"%lu\n", now);


	//Calculate different 
	unsigned long diff = now - load_jiffies;

	seq_printf(m, "Uptime since load: ");
	seq_printf(m, "%u\n", jiffies_to_msecs(diff));


    //Show acccess count
	/** 
	int val = atomic_read(&counter); //Atomic read, every reader will get a unique int number
	atomic_inc(&counter); 

	This actually could lead to multiple people having the asme value or the otherway around values being skipped 
	so the following is better
	*/
    int val = atomic_inc_return(&counter);

	seq_printf(m, "Access count: ");
	seq_printf(m,"%d\n", val);
	seq_printf(m, "\n");
	//Increment atomically. Honestly may have changed values at this point but doesnt matter since 
	// we are done reading it. 

	return 0;
}

static int lkp_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, lkp_info_show, NULL);
}

static const struct proc_ops lkp_info_ops = {
	.proc_open    = lkp_info_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int __init lkp_info_init(void)
{
	/* Record load-time jiffies */ 
	load_jiffies = jiffies;

	/* Create /proc/lkp_info using proc_create() */
	proc_entry = proc_create(
		"lkp_info", //The name of the file
		0444, //The privelleges. Everyone can read, no one can write
		NULL, //Parent directory: In this case we want the parent to be /proc so we pass NULL
		&lkp_info_ops //Links to avaialble operations for this proc file. 
	);

	/* Check return value of proc_create() */
	if (!proc_entry)
		return -ENOMEM; //If we dont have this file return no memory. The kernel uses negative 
	// Error codes for some reason?


	pr_info("module loaded\n");
	return 0;
}

static void __exit lkp_info_exit(void)
{
	/*Remove the /proc entry */

	proc_remove(proc_entry);
	pr_info("module unloaded\n");
}

module_init(lkp_info_init);
module_exit(lkp_info_exit);
