set title "Theory Timing of running n nodes"
set xlabel "p = number of cores"
set ylabel "t = timeElapsed (sec)"
set autoscale
set xrange [1:10]
set yrange [0.0:80.0]
#set autoscale
set term png
set grid
set output "theoryTime.png"
plot "plotTime.txt" using 1:4 title "Theory Time" with linespoints pt 4


