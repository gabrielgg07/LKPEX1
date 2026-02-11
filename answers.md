# Exercise 1: Answers

## Part C: Conceptual Questions (20 pts)

### Q1: GFP Flags and Context (4 pts)

The difference between these two flags is that GFP_KERNEL is used for normal context and GFP_ATOMIC is used for atomic context. This means that if you call kmalloc with GFP_KERNEL then it is possible that the allocator can sleep while waiting to reclaim memory, write pages to disk etc. GFP_ATOMIC means that the allocator must not sleep and is typically found in interrupts or while holding spin locks. The problem with using GFP_KERNEL if you hold a spinlock is that it is possible that the allocator goes to sleep while waiting to allocate or free memory, yet it holds a lock and causes deadlock on other parts of the kernel. 
### Q2: Safe Iteration (4 pts)

The difference between these two Macros is how they iterate over the list. In list_for_each_entry, you iterate over the list using the struct entry to obtain the next element. Essentially the next element is obtained from the current element. However, in list_for_each_entry_safe we store a pointer to the next element and it is no longer obtained through the current element. This is very important becuase if you were to use list_for_each_entry to delete the list we could see a variety of problems. Once you call list_del on entry->listitem and you kfree entry at that specific point, now when the macro tries to obtain the next element, not only is that entry unlinked from the list by list_del but you will also see an issue with use after free, meaning either of these function calls will break iteration. 

### Q3: Real-World Data Structure Usage (4 pts)

- **Linked list**: Uses linked lists in its wait queues and its run queues since most of the operations will just be append to end or remove from head and we are going sequentially along the list. This linked list provides O(1) insertion and deletion at the head or the tail of the list making it perfect for a subsystem like wait_queue_head
- **Hash table**: Hash tables are used by the netfilter connection tracking subystem since they can index multiple keys very quickly and provide O(1) lookup time for them. Each network connection is stored in the hashtable indexed by source,destination, ip, port, and protocol. This allows incoming packets to be indexed and match to an existing connection extremely quickly. 
- **Red-black tree**: These are used in memory managment subsystems such as mm/mmap.c. When a page fault occurs the kernel must first figure out if the Virtual address mapping is valif for this program. It can do so quickly by looking through the red vblack tree to find the valid mapping. 
- **XArray**: XArray is used for page caching lookups such as in mm/filemap.c to map file page indicies to specfic pages. An XArray is great here becuase it uses less memory then a typical array and still provides great effeciency with sparse indexing allowing you to find the pages of a file in the cache even if the file is extremely large. 
### Q4: Benchmark Analysis (4 pts)

Linked List - O(n)
Hash Table - O(1)
Red-Black Tree - O(logn)
XArray lookup - O(logn)

Looking at the metrics I measured. 

LKP Data Structure Benchmark (N=100)
=======================================
Insert (ns/op):
  Linked list:    48
  Hash table:     61
  Red-black tree: 71
  XArray:         70

Lookup (ns/op):
  Linked list:    62
  Hash table:     12
  Red-black tree: 26
  XArray:         6
1000  45 16 61 27  1220 52 41 5 
LKP Data Structure Benchmark (N=1000)
=======================================
Insert (ns/op):
  Linked list:    45
  Hash table:     16
  Red-black tree: 61
  XArray:         27

Lookup (ns/op):
  Linked list:    1220
  Hash table:     52
  Red-black tree: 41
  XArray:         5
5000  20 11 77 59  6674 508 61 7 


We can see that for 1000 items the lookup is 1200 ns or roughly a little over 1ms. Dropping it to 100 items its much smaller, .01ms. So somehwere between 100-1000 items is where it becomes impractical. This actually makes sense becuase since linked lists are being used for tasks that is a normal range for how nmany taks are going to be waiting in the ready/waiting queue. 

### Q5: Module Error Handling (4 pts)

(a) The kernel load immediately stops, exit is not called becuase the module never actually succesfully loaded. 

(b) If the proc pointer is Null and we try to remove it later we will have an issue that we may dereference a Nullpointer causing a potneital Kernel crash or panic and can crash the kernel. 

(c) The Goto pattern is used on init to make sure that all memory and objects is cleaned up in event of failure and is done in reverse order of allocation. For example below.

int __init my_init(void)
{
    procfile = proc_create(...);
    if (!proc)
        goto err_proc;

    buffer = kmalloc(...);
    if (!buffer)
        goto err_buffer;

    return 0;

err_buffer:
    proc_remove(procfile);
err_proc:
    return -ENOMEM;
}

We can see that if we have an error with proc we just to the error spot and return ENOMEM. If we have an error creating the buffer, we jump to that error and we make sure to remove the proc file. This is similar to just doing them inside the if statements however, this provides more scalibility becuase it better breaks it down into steps we can replicate or modify later if we change the init code. 


## Part D: AI Usage Reflection (5 pts)

### Tools used (1 pt)

ChatGPT

### How AI helped (1 pt)

AI was extremely helpful with this project, however, I mainly used it as a learning tool. I have already made Kernel modules before so I was familiar with that however, I have almost never heard of Kernel data structures so that was new to me. Before starting I found it extremely helpful to use AI to explain why there are so many of these Kernel data structures, what some of their definitions look like, and what they are used for. I also found it valuable that it could point me in the write direction such as which files to look at to find the function definitions that I would choose to use

### Limitations encountered (1 pt)

So AI can be very helpful however, I did find occasionally that some of the functions it would suggest would not be the best to use. For example when asking for more information on XArrays it did not exactly reccomend using XARray_destroy, which was actually the simplest option. I also found that often times when I would ask it to help me check over little pieces of code it would often severely overlook memory errors or memeory leaks. Becuase of both of these issues I found it very important to ensure I went into the kernel source code, found function definitions myself, and wrote the code and caught memory errors myself. 

### Learning impact (1 pt)

I think becasue I didnt really use the AI to code, it was significacntly more impactful in helping me learn rather than completing task. I probably could have asked it to complete large portions of the code for me but that would diminish my learning value and it would also probably cause significant bugs. Becuase I mainly just used it to explore concepts though I think I was able to learn the core principles of this project extremely quickly, and I was able to complete it quickly. 

### Suggestions (1 pt)

Overall usefulness (1-5): [5]

I think this was a great exercise, I appreciate the the RB-tree insert was already made for us and I understand that this is not a class about data structures but I think it would have been a good added challenge/learning moment if we had to implement that helper. 
