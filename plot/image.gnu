reset
set title "similarity matrix"
set xlabel "books"
set ylabel "books"
set palette defined (0 'white', 1 'black')
set xrange [0:8]
set yrange [0:8]
set autoscale
set cblabel 'Cosine Coefficient'
set term png
set grid
set output "image.png"
plot "data.txt"  matrix with image, 'data.txt'  matrix using 2 : 1 : (sprintf('%f', $3)) with labels font ',16'  title ' '
