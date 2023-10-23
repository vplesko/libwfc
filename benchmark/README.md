Quite a bit of effort has gone into performance improvements. If you're curious, you may read my [devlog](https://www.vplesko.com/posts/wfc/devlog_0.html) for this project - [\#2](https://www.vplesko.com/posts/wfc/devlog_2.html) and [\#4](https://www.vplesko.com/posts/wfc/devlog_4.html) are about optimizations.

This directory contains a benchmark test, with results in this file. This file also lists running times of the CLI tool for some inputs.

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
	avg=5.9566 min=5.9297 max=5.9958

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
	avg=284.1665 min=284.1041 max=284.1963
```

## CLI

Can be built with `make cli`.

```
time bin/cli external/samples/NotKnot.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m6.326s
0m6.340s
0m6.329s
```

```
time bin/cli external/samples/Cat.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m36.443s
0m36.363s
0m36.513s
```

```
time bin/cli external/samples/BrownFox.png -n 5 -w 50 -h 50 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m47.951s
0m47.956s
0m47.951s
```
