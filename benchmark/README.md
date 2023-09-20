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
        avg=6.4072 min=6.2960 max=6.5170

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
        avg=4.8008 min=4.7960 max=4.8090
```

## CLI

Can be built with `make cli`.

```
.\bin\cli.exe .\external\samples\NotKnot.png -n 3 -w 256 -h 256 -o .\bin\output.png -flip -rot
```

Result (multiple runs; skip on backtrack or fail):

```
0:06.58
0:06.63
0:06.60
```

```
.\bin\cli.exe .\external\samples\Cat.png -n 3 -w 128 -h 128 -o .\bin\output.png -flip -rot
```

Result (multiple runs; skip on backtrack or fail):

```
0:08.49
0:08.73
0:08.54
```

```
.\bin\cli.exe .\external\samples\BrownFox.png -n 5 -w 50 -h 50 -o .\bin\output.png -flip -rot
```

Result (multiple runs; skip on backtrack or fail):

```
0:15.04
0:14.57
0:17.18
```
