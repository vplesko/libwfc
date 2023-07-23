#include <stdlib.h>

// @TODO struct for args
// @TODO rand seed as arg
int parseArgs(int argc, char *argv[],
    const char **imagePath, int *wfcN, int *dstW, int *dstH) {
    if (argc < 5) return -1;

    *imagePath = argv[1];

    long l;
    char *end;

    l = strtol(argv[2], &end, 0);
    if (*end != '\0') return -1;
    *wfcN = (int)l;

    l = strtol(argv[3], &end, 0);
    if (*end != '\0') return -1;
    *dstW = (int)l;

    l = strtol(argv[4], &end, 0);
    if (*end != '\0') return -1;
    *dstH = (int)l;

    return 0;
}
