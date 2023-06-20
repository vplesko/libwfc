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
image=external/samples/Angular.png repeats=5 args={n=3 dstW=64 dstH=64}
        ....o
        avg=2.6057 min=2.4360 max=2.8737

image=benchmark/test.txt repeats=5 args={n=5 dstW=120 dstH=120}
        ....o
        avg=11.5774 min=11.3916 max=11.7893
```
