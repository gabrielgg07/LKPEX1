set terminal pdfcairo size 10,4 font "Palatino,12"
set output "bench_results.pdf"
set multiplot layout 1,2

set xlabel "Number of Entries (N)"
set logscale x
set xtics (100, 1000, 5000, 10000, 50000)
set grid ytics
set key top left

# (a) Insert
set ylabel "Insert Time (ns/op)"
set title "(a) Insert Performance"
plot "bench_data.txt" using 1:2 w lp title "Linked List" pt 7, \
     ""               using 1:3 w lp title "Hash Table" pt 5, \
     ""               using 1:4 w lp title "RB-Tree" pt 9, \
     ""               using 1:5 w lp title "XArray" pt 13

# (b) Lookup
set ylabel "Lookup Time (ns/op)"
set title "(b) Lookup Performance"
plot "bench_data.txt" using 1:6 w lp title "Linked List" pt 7, \
     ""               using 1:7 w lp title "Hash Table" pt 5, \
     ""               using 1:8 w lp title "RB-Tree" pt 9, \
     ""               using 1:9 w lp title "XArray" pt 13

unset multiplot
