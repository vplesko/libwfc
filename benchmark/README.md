## Environment

 - Windows 10
 - Intel Core i7-8750H
 - clang 16
 - gcc 13.1.0
 - Visual C++ (x64) 14.30.30708

## Benchmark test

```
$ make WIN=1 VC=1 benchmark
clang -std=c99 -Wall -Wextra -pedantic -Werror -I./ -Ishared -Iexternal/lib -g -fno-omit-frame-pointer -D_CRT_SECURE_NO_WARNINGS -O3 -mavx2 benchmark/main.c -o bin/benchmark/main  bin/lib/stb_image_impl.obj
bin/benchmark/main
```

Result:

```
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=256 dstH=256}
        avg=6.7566 min=6.6260 max=6.8740

input=benchmark/test.txt repeats=5 args={n=5 opt=7 dstW=100 dstH=100}
        avg=9.0612 min=9.0190 max=9.1300
```

## CLI

```
$ make WIN=1 VC=1 cli

$ time .\bin\cli.exe .\external\samples\NotKnot.png -n 3 -w 128 -h 128 -o .\bin\output.png -flip -rot
```

Result (multiple runs):

```
0:01.28
0:01.26
0:01.24
```
