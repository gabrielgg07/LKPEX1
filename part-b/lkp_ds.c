/*
 * lkp_ds.c - Kernel Data Structures Module (Exercise 1, Part B)
 *
 * Stores integers in four data structures (linked list, hash table,
 * red-black tree, XArray) and exposes them via /proc/lkp_ds.
 * Includes a scalability benchmark reported via /proc/lkp_ds_bench.
 */
#define pr_fmt(fmt) "lkp: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/rbtree.h>
#include <linux/xarray.h>
#include <linux/ktime.h>
#include <linux/random.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("LKP: Data Structure Benchmark");

static char *int_str;
module_param(int_str, charp, 0444);
MODULE_PARM_DESC(int_str, "A comma-separated list of integers");

static int bench_size = 1000;
module_param(bench_size, int, 0444);
MODULE_PARM_DESC(bench_size, "Number of entries for the benchmark");

/*
 * Entry struct: embeds nodes for all four data structures.
 * Each integer from int_str creates ONE allocation that is inserted
 * into all four structures simultaneously.
 */
struct my_entry {
	int value;
	struct list_head list;       /* linked list */
	struct hlist_node hnode;     /* hash table */
	struct rb_node node;         /* red-black tree */
	/* XArray stores a pointer to this entry; no embedded node needed */
};

/* --- Correctness data structures (populated from int_str) --- */
static LIST_HEAD(my_list);
static DEFINE_HASHTABLE(my_htable, 4);   /* 2^4 = 16 buckets */
static struct rb_root my_tree = RB_ROOT;
static DEFINE_XARRAY(my_xarray);
static unsigned long xa_next_index;      /* tracks next XArray index */

static struct proc_dir_entry *proc_ds;
static struct proc_dir_entry *proc_bench;

/* --- Benchmark results (filled on module load) --- */
static u64 bench_insert_ns[4];  /* list, hash, rbtree, xarray */
static u64 bench_lookup_ns[4];
static int bench_n;

/* ===================================================================
 * Red-black tree insertion helper (provided)
 * =================================================================== */

static void insert_rbtree(struct rb_root *root, struct my_entry *new)
{
	struct rb_node **link = &root->rb_node;
	struct rb_node *parent = NULL;
	struct my_entry *entry;

	while (*link) {
		parent = *link;
		entry = rb_entry(parent, struct my_entry, node);

		if (new->value < entry->value)
			link = &parent->rb_left;
		else
			link = &parent->rb_right;
	}

	rb_link_node(&new->node, parent, link);
	rb_insert_color(&new->node, root);
}


/* ===================================================================
 * Correctness: store/display/free int_str values
 * =================================================================== */

/*
 * TODO: Implement store_value()
 * Allocate one my_entry, set its value, and insert it into all 4 structures:
 *   - list: list_add_tail(&entry->list, &my_list)
 *   - hash: hash_add(my_htable, &entry->hnode, val)
 *   - rbtree: insert_rbtree(&my_tree, entry)
 *   - xarray: xa_store(&my_xarray, xa_next_index++, entry, GFP_KERNEL)
 * Return 0 on success, -ENOMEM on allocation failure.
 */
static int store_value(int val)
{
	//Allocate an entry
	struct my_entry *e;

	e = kmalloc(sizeof(*e), GFP_KERNEL);
	if (!e)
		return -ENOMEM;
	//Fill in the value
	e->value = val;


	// Use herlper functions directly provided by list.h, they use write_once read_once to ensure correctness.
	// Importantly when reading the file we need to look for functions that are not internal meaning they dont start with
	// __Function_name
	// Kernel Helper for finding the tail (mylist.prev), linking new list_head, and updating pointers	
	list_add_tail(&e->list, &my_list);
	xa_store(&my_xarray, xa_next_index++, e, GFP_KERNEL);
	hash_add(my_htable, &e->hnode, val);
	
	RB_CLEAR_NODE(&e->node);
	insert_rbtree(&my_tree, e);
	return 0;
}

/*
 * Implementation of parse params
 */
static int parse_params(void)
{
	//Track value and error
	int val, err = 0;
	char *p, *orig, *params; //*p is the pointer to the char we attempt to convert to an int
	// *orig is the orignal allocated string we will free later
	// *params is the pointer we iterate over


	//Realisitically is already been checked in init but no harm in double checking
	params = kstrdup(int_str, GFP_KERNEL);
	if (!params)
		return -ENOMEM;
	orig = params;

	while ((p = strsep(&params, ",")) != NULL) {
		if (!*p)
			continue;

		err = kstrtoint(p, 0, &val);
		if (err)
			break;

		err = store_value(val);
		if (err)
			break;
	}

	kfree(orig);
	return err;

}

/* --- /proc/lkp_ds show --- */
static int lkp_ds_show(struct seq_file *m, void *v)
{
	/* TODO: Iterate each structure and print values, comma-separated.
	 *
	 * Expected format (one line per structure):
	 *   Linked list:    1, 2, 3, 4, 5
	 *   Hash table:     3, 4, 1, 2, 5
	 *   Red-black tree: 1, 2, 3, 4, 5
	 *   XArray:         1, 2, 3, 4, 5
	 *
	 * Iteration macros:
	 *   list: list_for_each_entry(entry, &my_list, list)
	 *   hash: hash_for_each(my_htable, bkt, entry, hnode)
	 *   rbtree: for (rb = rb_first(&my_tree); rb; rb = rb_next(rb))
	 *           then rb_entry(rb, struct my_entry, node)
	 *   xarray: xa_for_each(&my_xarray, index, entry)
	 *
	 * Note: hash table output order depends on the kernel's hash function
	 * and will differ from the example. This is expected.
	 */

	// Create an iterable entry
	struct my_entry *e;
	// First we go over the list
    seq_printf(m, "Linked List: ");

	list_for_each_entry(e, &my_list, list /** This is the field name in the first struct we passed to get the pointer */ ) {
		seq_printf(m, "%d, ", e->value);
	}
    seq_printf(m, "\n");

	// We can use the same entry 

	int bkt; //This is used to just see which bucket we are on when iterating the has table. Not 
	//neccesarry for now but could be useful for debugging purposes

	// No we iterate hash
	seq_printf(m, "Hash Table: ");
	hash_for_each(my_htable, bkt, e, hnode) {
		seq_printf(m, "%d, ", e->value);
	}
	seq_printf(m, "\n");

	struct rb_node *rb; //we need to get the rbnode to move over
    seq_printf(m, "Red-Black tree: ");
	for (rb = rb_first(&my_tree); rb; rb = rb_next(rb)) {
		struct my_entry *entry = rb_entry(rb, struct my_entry, node);

		seq_printf(m, "%d, ", entry->value);
	}
	seq_printf(m, "\n");

	//use the same entry again?
	seq_printf(m, "XArray: ");
	unsigned long index;
	xa_for_each(&my_xarray, index, e) {
		seq_printf(m, "%d, ", e->value);
	}
	seq_printf(m, "\n");
	return 0;
}

static int lkp_ds_open(struct inode *inode, struct file *file)
{
	return single_open(file, lkp_ds_show, NULL);
}

static const struct proc_ops lkp_ds_ops = {
	.proc_open    = lkp_ds_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

/* ===================================================================
 * Benchmark (Part B.3)
 * =================================================================== */

/* --- /proc/lkp_ds_bench show --- */
static int lkp_bench_show(struct seq_file *m, void *v)
{
	seq_printf(m, "LKP Data Structure Benchmark (N=%d)\n", bench_n);
	seq_printf(m, "=======================================\n");
	seq_printf(m, "Insert (ns/op):\n");
	seq_printf(m, "  Linked list:    %llu\n", bench_insert_ns[0]);
	seq_printf(m, "  Hash table:     %llu\n", bench_insert_ns[1]);
	seq_printf(m, "  Red-black tree: %llu\n", bench_insert_ns[2]);
	seq_printf(m, "  XArray:         %llu\n", bench_insert_ns[3]);
	seq_printf(m, "\n");
	seq_printf(m, "Lookup (ns/op):\n");
	seq_printf(m, "  Linked list:    %llu\n", bench_lookup_ns[0]);
	seq_printf(m, "  Hash table:     %llu\n", bench_lookup_ns[1]);
	seq_printf(m, "  Red-black tree: %llu\n", bench_lookup_ns[2]);
	seq_printf(m, "  XArray:         %llu\n", bench_lookup_ns[3]);
	return 0;
}

static int lkp_bench_open(struct inode *inode, struct file *file)
{
	return single_open(file, lkp_bench_show, NULL);
}

static const struct proc_ops lkp_bench_ops = {
	.proc_open    = lkp_bench_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

/*
 * TODO: Implement run_benchmark()
 *
 * IMPORTANT: Use SEPARATE data structures for the benchmark so that
 * benchmark data does not pollute the correctness data from int_str.
 * Declare local list/hashtable/rbtree/xarray at the top of this function
 * (or as file-scope statics with bench_ prefix).
 *
 * Steps:
 * 1. Allocate an array of bench_size random ints (use get_random_u32())
 * 2. For each data structure:
 *    a. Time inserting all bench_size entries (kmalloc + insert)
 *    b. Store total_ns / bench_size in bench_insert_ns[]
 * 3. For each data structure, time bench_size random lookups:
 *    - List: traverse to find a value (O(n))
 *    - Hash: hash_for_each_possible with value as key (O(1) avg)
 *    - RB-tree: binary search by value (O(log n))
 *    - XArray: xa_load by index (O(1))
 *    Store total_ns / bench_size in bench_lookup_ns[]
 * 4. Free all benchmark entries and auxiliary arrays
 */
static int run_benchmark(void)
{
	u64 start, elapsed;
	bench_n = bench_size;
	//Create bench size randoms;
	// Here i was originally using a normasl array with [] but thats bad becuase with variables it could be too
	//big for kernel stack and that could. be very bad
	u32 *random;

	random = kmalloc_array(bench_n, sizeof(u32), GFP_KERNEL);
	if (!random)
		return -ENOMEM;
	for(int i = 0; i < bench_n; i++ ){
		u32 val = get_random_u32();
		int range = 1000000;
        int key = val % range;  /* limit to desired range */
		random[i] = key;
	}

	//Time the list
	LIST_HEAD(bench_list);


	start = ktime_get_ns();
	for(int i = 0; i < bench_n; i++ ){
		struct my_entry *e;
		e = kmalloc(sizeof(*e), GFP_KERNEL);
		if (!e)
			return -ENOMEM;
		//Fill in the value
		e->value = random[i];
		list_add_tail(&e->list, &bench_list);
	}

	elapsed = ktime_get_ns() - start;
	bench_insert_ns[0] = elapsed / bench_n;

	DECLARE_HASHTABLE(bench_htable, 4);

	start = ktime_get_ns();
	for(int i = 0; i < bench_n; i++ ){
		struct my_entry *he;
		he = kmalloc(sizeof(*he), GFP_KERNEL);
		if (!he)
			return -ENOMEM;
		//Fill in the value
		he->value = random[i];
		
		hash_add(bench_htable, &he->hnode, he->value);
	}
	elapsed = ktime_get_ns() - start;
	bench_insert_ns[1] = elapsed / bench_n;

	struct rb_root bench_tree = RB_ROOT;

	start = ktime_get_ns();
	for(int i = 0; i < bench_n; i++ ){
		struct my_entry *re;
		re = kmalloc(sizeof(*re), GFP_KERNEL);
		if (!re)
			return -ENOMEM;
		//Fill in the value
		re->value = random[i];
		
		RB_CLEAR_NODE(&re->node);
		insert_rbtree(&bench_tree, re);
	}
	elapsed = ktime_get_ns() - start;
	bench_insert_ns[2] = elapsed / bench_n;

	DEFINE_XARRAY(bench_xarray);
	unsigned long bench_xa_index = 0;

	start = ktime_get_ns();
	for(int i = 0; i < bench_n; i++ ){
		struct my_entry *xe;
		xe = kmalloc(sizeof(*xe), GFP_KERNEL);
		if (!xe)
			return -ENOMEM;
		//Fill in the value
		xe->value = random[i];


		xa_store(&bench_xarray, bench_xa_index++, xe, GFP_KERNEL);
	}
	elapsed = ktime_get_ns() - start;
	bench_insert_ns[3] = elapsed / bench_n;



	start = ktime_get_ns();
	for(int i = 0; i < bench_n; i++ ){
		struct my_entry *e;
		//bool found;
		int target = random[i];
		list_for_each_entry(e, &bench_list, list) {
			if (e->value == target) {
				//found = true;
				break;
			}
		}

	}
	elapsed = ktime_get_ns() - start;
	bench_lookup_ns[0] = elapsed / bench_n;


	start = ktime_get_ns();
	
	for (int i = 0; i < bench_n; i++) {
		struct my_entry *he;
		int target = random[i];

		hash_for_each_possible(bench_htable, he, hnode, target) {
			if (he->value == target)
				break;
		}
	}
	elapsed = ktime_get_ns() - start;
	bench_lookup_ns[1] = elapsed / bench_n;

	start = ktime_get_ns();

	for (int i = 0; i < bench_n; i++) {
		struct rb_node *node = bench_tree.rb_node;
		int target = random[i];

		while (node) {
			struct my_entry *e = rb_entry(node, struct my_entry, node);

			if (target < e->value)
				node = node->rb_left;
			else if (target > e->value)
				node = node->rb_right;
			else
				break;   // found
		}
	}

	elapsed = ktime_get_ns() - start;
	bench_lookup_ns[2] = elapsed / bench_n;



	start = ktime_get_ns();

	for (int i = 0; i < bench_n; i++) {
		xa_load(&bench_xarray, i);
	}

	elapsed = ktime_get_ns() - start;
	bench_lookup_ns[3] = elapsed / bench_n;

	//Must Free All!
	//Free the list 
	struct my_entry *list_entry;
	struct my_entry *list_tmp;
	list_for_each_entry_safe(list_entry, list_tmp, &bench_list, list) {
		list_del(&list_entry->list);
		kfree(list_entry);
	}

	//Free the hashtable
	struct my_entry *he;
	struct hlist_node *tmp_hnode;
	int bkt;

	hash_for_each_safe(bench_htable, bkt, tmp_hnode, he, hnode) {
		hash_del(&he->hnode);
		kfree(he);
	}

	//Free the rb tree
	struct rb_node *node;
	for (node = rb_first(&bench_tree); node; ) {
		struct my_entry *re = rb_entry(node, struct my_entry, node);
		node = rb_next(node);
		rb_erase(&re->node, &bench_tree);
		kfree(re);
	}

	unsigned long index_delete;
	struct my_entry *xe;

	xa_for_each(&bench_xarray, index_delete, xe) {
		xa_erase(&bench_xarray, index_delete);
		kfree(xe);
	}
	xa_destroy(&bench_xarray);




	kfree(random);


	return 0;
}

/* ===================================================================
 * Cleanup helpers
 * =================================================================== */

/*
 * TODO: Implement free_all()
 *
 * Since each my_entry is inserted into ALL four structures, you must
 * remove it from each structure but only call kfree() ONCE per entry.
 *
 * Recommended approach: iterate one structure (e.g., linked list) with
 * the _safe variant, remove from list, hash, rbtree, and xarray, then
 * kfree the entry. Alternatively, iterate xarray with xa_for_each(),
 * and for each entry: list_del, hash_del, rb_erase, xa_erase, kfree.
 */
static void free_all(void)
{
	/*Going to use a similar structure iterating over the list and deleting each entry one by one*/

	//Create a position entry
	struct my_entry *e;
	struct my_entry *tmp; //This becomes the next node we are going to, so we load e
	// save e->next to tmp and then load that after we free e so its safe

	/**
	 * We need to use
	 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
	 * @pos:	the type * to use as a loop cursor.
	 * @n:		another type * to use as temporary storage
	 * @head:	the head for your list.
	 * @member:	the name of the list_head within the struct.
	 */
	// IMPORTANT this only returns nodes that are not sentinel head node. This is good beaxcuse the ssentinel
	// is not wrapped in an entry


	xa_destroy(&my_xarray);
	list_for_each_entry_safe(e, tmp, &my_list, list) {
		// delete from each structure and then free

		/**
		 * list_del - deletes entry from list.
		 * @entry: the element to delete from the list.
		 * Note: list_empty() on entry does not return true after this, the entry is
		 * in an undefined state.
		 */
		list_del(&e->list);
		hash_del(&e->hnode);
		rb_erase(&e->node, &my_tree);
		kfree(e);

	}
}

/* ===================================================================
 * Module init / exit
 * =================================================================== */

static int __init lkp_ds_init(void)
{
	int err;

	if (!int_str) {
		pr_err("missing 'int_str' parameter\n");
		return -EINVAL;
	}

	err = parse_params();
	if (err) {
		pr_err("failed to parse int_str\n");
		return err;
	}

	proc_ds = proc_create("lkp_ds", 0444, NULL, &lkp_ds_ops);
	if (!proc_ds) {
		pr_err("failed to create /proc/lkp_ds\n");
		free_all();
		return -ENOMEM;
	}

	proc_bench = proc_create("lkp_ds_bench", 0444, NULL, &lkp_bench_ops);
	if (!proc_bench) {
		pr_err("failed to create /proc/lkp_ds_bench\n");
		proc_remove(proc_ds);
		free_all();
		return -ENOMEM;
	}

	run_benchmark();

	pr_info("module loaded (int_str=%s, bench_size=%d)\n",
		int_str, bench_size);
	return 0;
}

static void __exit lkp_ds_exit(void)
{
	proc_remove(proc_bench);
	proc_remove(proc_ds);
	free_all();
	pr_info("module unloaded\n");
}

module_init(lkp_ds_init);
module_exit(lkp_ds_exit);
