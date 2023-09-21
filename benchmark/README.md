## Environment

 - Windows 10
 - Intel Core i7-8750H
 - clang 16
 - gcc 13.1.0
 - Visual C++ (x64) 14.30.30708

## Benchmark test

Can be run with `make benchmark`.

Result:

```
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=256 dstH=256}
        avg=5.5866 min=5.5340 max=5.6560

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
        avg=4.7580 min=4.7490 max=4.7680
```

## CLI

Can be built with `make cli`.

```
time .\bin\cli.exe .\external\samples\NotKnot.png -n 3 -w 256 -h 256 -o .\bin\output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0:05.98
0:05.92
0:05.90
```

```
time .\bin\cli.exe .\external\samples\Cat.png -n 3 -w 256 -h 256 -o .\bin\output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0:36.12
0:36.60
0:36.46
```

```
time .\bin\cli.exe .\external\samples\BrownFox.png -n 5 -w 50 -h 50 -o .\bin\output.png -flip -rot -seed 1600001001
```

Result (multiple runs):

```
0:14.65
0:14.63
0:14.63
```
