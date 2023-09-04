## Environment

 - Windows 10
 - Intel Core i7-4771
 - clang 16
 - gcc 12.2.0
 - Visual C++ (x64) 14.36.32532

## Benchmark test

```
$ make WIN=1 VC=1 benchmark
clang -std=c99 -Wall -Wextra -pedantic -Werror -I./ -Ishared -Iexternal/lib -g -fno-omit-frame-pointer -D_CRT_SECURE_NO_WARNINGS -O3 -mavx2 benchmark/main.c -o bin/benchmark/main  bin/lib/stb_image_impl.obj
bin/benchmark/main
```

Result:

```
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=128 dstH=128}
        avg=4.1814 min=4.1420 max=4.2170

input=benchmark/test.txt repeats=5 args={n=5 opt=3 dstW=80 dstH=80}
        avg=16.2622 min=16.0820 max=16.4480
```

## CLI

```
$ make WIN=1 VC=1 cli
clang -std=c99 -Wall -Wextra -pedantic -Werror -I./ -Ishared -Iexternal/lib -g -fno-omit-frame-pointer -D_CRT_SECURE_NO_WARNINGS -O3 -mavx2 shared/stb_image_impl.c -c -o bin/lib/stb_image_impl.obj
clang -std=c99 -Wall -Wextra -pedantic -Werror -I./ -Ishared -Iexternal/lib -g -fno-omit-frame-pointer -D_CRT_SECURE_NO_WARNINGS -O3 -mavx2 cli/main.c -o bin/cli.exe  bin/lib/stb_image_impl.obj

$ time .\bin\cli.exe .\external\samples\NotKnot.png -n 3 -w 128 -h 128 -o .\bin\output.png -flip -rot
```

Result:

```
[...]
0.01user 0.00system 0:15.12elapsed 0%CPU (0avgtext+0avgdata 4888maxresident)k
0inputs+0outputs (1324major+0minor)pagefaults 0swaps

[...]
0.00user 0.00system 0:15.71elapsed 0%CPU (0avgtext+0avgdata 4892maxresident)k
0inputs+0outputs (1295major+0minor)pagefaults 0swaps

[...]
0.01user 0.00system 0:15.48elapsed 0%CPU (0avgtext+0avgdata 4900maxresident)k
0inputs+0outputs (1296major+0minor)pagefaults 0swaps
```
