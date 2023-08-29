Environment:

 - Windows 10
 - Intel Core i7-4771
 - clang 16
 - gcc 12.2.0
 - Visual C++ (x64) 14.36.32532

Compilation:

`clang -std=c99 [...] -g -fno-omit-frame-pointer -O3 -mavx2 [...] -lm`

Result:

```
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=64 dstH=64}
        ....o
        avg=7.8390 min=7.4560 max=8.1060

input=benchmark/test.txt repeats=5 args={n=5 opt=3 dstW=80 dstH=80}
        ....o
        avg=16.7020 min=16.5230 max=16.8280
```