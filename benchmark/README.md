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
	avg=5.2825 min=5.2288 max=5.3409

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
	avg=4.2445 min=4.2327 max=4.2597
```

## CLI

Can be built with `make cli`.

```
time bin/cli external/samples/NotKnot.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m5.388s
0m5.401s
0m5.372s
```

```
time bin/cli external/samples/Cat.png -n 3 -w 256 -h 256 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m24.374s
0m24.462s
0m24.447s
```

```
time bin/cli external/samples/BrownFox.png -n 5 -w 50 -h 50 -o bin/output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0m10.383s
0m10.412s
0m10.420s
```
