set title "Timing data of Actual running time on n nodes"
set xlabel "p = number of cores"
set ylabel "t = timeElapsed (sec)"
set autoscale
set xrange [1:10]
set yrange [0.0:80.0]
#set autoscale
set term png
set grid
set output "actualTime.png"
plot "plotTime.txt" using 1:2 title "Actual Time" with linespoints pt 4


