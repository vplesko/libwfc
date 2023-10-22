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
	avg=5.9925 min=5.9701 max=6.0293

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
	avg=306.6342 min=306.4930 max=306.7443
```

## CLI

Can be built with `make cli`.

```
time bin/cli external/samples/NotKnot.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m6.397s
0m6.413s
0m6.387s
```

```
time bin/cli external/samples/Cat.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m38.559s
0m38.276s
0m38.292s
```

```
time bin/cli external/samples/BrownFox.png -n 5 -w 50 -h 50 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m51.367s
0m51.316s
0m51.350s
```
