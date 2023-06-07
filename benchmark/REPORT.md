Environment:

 - Debian 11
 - Intel Core i7-8750H
 - clang 17
 - gcc 10.2.1
 - glibc 2.31

Compilation:

`clang -std=c99 [...] -g -fno-omit-frame-pointer -O3 -mavx2 [...] -lm`

Result:

```
image=../external/samples/Angular.png repeats=5 args={n=3 dstW=64 dstH=64}
        ....o
        avg=9.7969 min=8.4966 max=10.2870

image=test.txt repeats=5 args={n=5 dstW=120 dstH=120}
        ....o
        avg=16.3255 min=16.0138 max=16.5205
```
