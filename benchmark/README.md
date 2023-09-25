Quite a bit of effort has gone into performance improvements. If you're curious, you may read my [devlog](https://www.vplesko.com/posts/wfc/devlog_0.html) for this project - [\#2](https://www.vplesko.com/posts/wfc/devlog_2.html) is about optimizations.

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
	avg=3.9972 min=3.9755 max=4.0220

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
	avg=4.1923 min=4.1849 max=4.2048
```

## CLI

Can be built with `make cli`.

```
time bin/cli external/samples/NotKnot.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m4.143s
0m4.135s
0m4.148s
```

```
time bin/cli external/samples/Cat.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m23.398s
0m23.395s
0m23.325s
```

```
time bin/cli external/samples/BrownFox.png -n 5 -w 50 -h 50 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m10.292s
0m10.297s
0m10.310s
```
