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
        avg=3.4732 min=3.4410 max=3.5080

input=benchmark/test.txt repeats=5 args={n=5 opt=3 dstW=80 dstH=80}
        avg=15.4406 min=15.1820 max=15.8140
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
0.00user 0.03system 0:04.44elapsed 0%CPU (0avgtext+0avgdata 4852maxresident)k
0inputs+0outputs (1309major+0minor)pagefaults 0swaps

[...]
0.00user 0.00system 0:04.33elapsed 0%CPU (0avgtext+0avgdata 4852maxresident)k
0inputs+0outputs (1280major+0minor)pagefaults 0swaps

[...]
0.00user 0.00system 0:04.39elapsed 0%CPU (0avgtext+0avgdata 4848maxresident)k
0inputs+0outputs (1280major+0minor)pagefaults 0swaps
```
