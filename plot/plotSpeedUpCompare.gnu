set title "SpeedUp compare on n nodes"
set xlabel "p = number of cores"
set ylabel "s = speed up"
set autoscale
set xrange [1:10]
set yrange [0.0:10.0]
#set autoscale
set term png
set grid
set output "SpeedUpCompare.png"
plot "plotTime.txt" using 1:3 title "Actual SpeedUp" with linespoints pt 4,\
"plotTime.txt" using 1:5 title "Theory SpeedUp" with linespoints pt 5


