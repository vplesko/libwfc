#include <stdlib.h>

struct Args {
    const char *imagePath;
    int wfcN;
    int dstW, dstH;
};

// @TODO rand seed as arg
int parseArgs(int argc, char *argv[], struct Args *args) {
    if (argc < 5) return -1;

    args->imagePath = argv[1];

    long l;
    char *end;

    l = strtol(argv[2], &end, 0);
    if (*end != '\0') return -1;
    args->wfcN = (int)l;

    l = strtol(argv[3], &end, 0);
    if (*end != '\0') return -1;
    args->dstW = (int)l;

    l = strtol(argv[4], &end, 0);
    if (*end != '\0') return -1;
    args->dstH = (int)l;

    return 0;
}
