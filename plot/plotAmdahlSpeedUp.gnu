set title "Theory running speed up on  n nodes"
set xlabel "p = number of cores"
set ylabel "s = theory speed up"
set autoscale
set xrange [1:10]
set yrange [0.0:10.0]
#set autoscale
set term png
set grid
set output "theorySpeedUp.png"
plot "plotTime.txt" using 1:5 title "Amdahl SpeedUp" with linespoints pt 4


