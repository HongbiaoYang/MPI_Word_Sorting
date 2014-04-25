set title "Actual SpeedUp on n nodes"
set xlabel "p = number of cores"
set ylabel "s = speed Up"
set autoscale
set xrange [1:10]
set yrange [0.0:10]
#set autoscale
set term png
set grid
set output "actualSpeedUp.png"
plot "plotTime.txt" using 1:3 title "Actual SpeedUp" with linespoints pt 4


