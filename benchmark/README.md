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
input=external/samples/NotKnot.png repeats=5 args={n=3 opt=7 dstW=128 dstH=128}
        avg=2.2066 min=2.1810 max=2.2300

input=benchmark/test.txt repeats=5 args={n=5 opt=3 dstW=80 dstH=80}
        avg=11.3184 min=11.2960 max=11.3680
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
0.00user 0.00system 0:02.89elapsed 0%CPU (0avgtext+0avgdata 4848maxresident)k
0inputs+0outputs (1309major+0minor)pagefaults 0swaps

[...]
0.00user 0.01system 0:02.85elapsed 0%CPU (0avgtext+0avgdata 4848maxresident)k
0inputs+0outputs (1280major+0minor)pagefaults 0swaps

[...]
0.00user 0.01system 0:02.85elapsed 0%CPU (0avgtext+0avgdata 4848maxresident)k
0inputs+0outputs (1281major+0minor)pagefaults 0swaps
```
