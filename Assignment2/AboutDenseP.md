# About Dense_P
compile:  
```
mpicc -std=c99 -g -O3 -o dense dense_p.c -lm
```
run:  
```
mpirun -np 4 dense input4.txt output4.txt
```
input4.txt:  
```
4
1 2 3 4
5 6 7 8
9 10 11 12
13 14 15 16
17 18 19 20
21 22 23 24
25 26 27 28
29 30 31 32
```
output4.txt:  
```
250.000000 260.000000 270.000000 280.000000 
618.000000 644.000000 670.000000 696.000000 
986.000000 1028.000000 1070.000000 1112.000000 
1354.000000 1412.000000 1470.000000 1528.000000 
```
check:  
![image](https://user-images.githubusercontent.com/65893273/116759097-1f074400-aa44-11eb-81bb-b326712d3288.png)
![image](https://user-images.githubusercontent.com/65893273/116759078-0a2ab080-aa44-11eb-815c-d90fd8332dcf.png)

Link here:  
https://matrix.reshish.com/zh/multCalculation.php

# NOTE
There is something wrong with serial code!
