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
        avg=0.6786 min=0.6440 max=0.6990

input=benchmark/test.txt repeats=5 args={n=5 opt=3 dstW=80 dstH=80}
        avg=16.2276 min=16.0900 max=16.3120
```
