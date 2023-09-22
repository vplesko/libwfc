## Environment

 - Debian 11
 - Intel Core i7-8750H
 - clang 17
 - gcc 10.2.1
 - glibc 2.31

## Benchmark test

Can be run with `make benchmark`.

Result:

```
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=256 dstH=256}
	avg=5.4413 min=5.4048 max=5.5042

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
	avg=4.4344 min=4.4230 max=4.4582
```

## CLI

Can be built with `make cli`.

```
time bin/cli external/samples/NotKnot.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m5.585s
0m5.592s
0m5.602s
```

```
time bin/cli external/samples/Cat.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m28.983s
0m28.753s
0m28.758s
```

```
time bin/cli external/samples/BrownFox.png -n 5 -w 50 -h 50 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m10.335s
0m10.384s
0m10.335s
```
