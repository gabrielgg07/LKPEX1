# Exercise 1: Kernel Modules & Data Structures

CS 5264/4224 & ECE 5414/4414 - Linux Kernel Programming - Spring 2026

## Directory Layout

```
ex1/
    part-a/
        lkp_hello/          A.1: Hello LKP Module (10 pts)
            lkp_hello.c         Skeleton - add count parameter and loop
            Makefile
        lkp_info/            A.2: /proc Interface Module (15 pts)
            lkp_info.c          Skeleton - add jiffies, uptime, access count
            Makefile
    part-b/
        lkp_ds.c             B.1-B.3: Data structures + benchmark (50 pts)
        Makefile
        bench.sh              Benchmark data collection script
        bench_data.txt        (you create this - measurement data)
        bench_results.pdf     (you create this - performance plots)
        plot_bench.gp         Gnuplot template
        plot_bench.py         Matplotlib template (use either one)
    answers.md               Part C: Conceptual questions (25 pts)
    time.txt                 Time tracking
    README.md                This file
```

## Quick Start

```bash
# Build and test Part A.1
cd part-a/lkp_hello
make
sudo insmod lkp_hello.ko
dmesg | grep "lkp:"
sudo rmmod lkp_hello

# Build and test Part B
cd part-b
make
sudo insmod lkp_ds.ko int_str="1,2,3,4,5"
cat /proc/lkp_ds
cat /proc/lkp_ds_bench
sudo rmmod lkp_ds
```

## Submission

Push all code and deliverables to your GitHub Classroom repository.
See the full specification (PDF) for detailed requirements.
# LKPEX1
