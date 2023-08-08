struct WfcWrapper {
    int len, cap;
    struct wfc_State **states;
    int counter;

    int bytesPerPixel;
};

int wfcInit(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH,
    struct WfcWrapper *wfc) {
    wfc->len = 0;
    wfc->cap = 50;
    wfc->states = malloc((size_t)wfc->cap * sizeof(*wfc->states));

    struct wfc_State *state = wfc_init(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH);
    if (state == NULL) {
        free(wfc->states);
        wfc->states = NULL;

        return -1;
    }
    wfc->states[wfc->len++] = state;

    wfc->counter = 0;

    wfc->bytesPerPixel = bytesPerPixel;

    return 0;
}

int wfcStatus(struct WfcWrapper *wfc) {
    return wfc_step(wfc->states[wfc->len - 1]);
}

int wfcStep(struct WfcWrapper *wfc) {
    int status = wfc_step(wfc->states[wfc->len - 1]);
    if (status != 0) return status;

    if (wfc->len < wfc->cap) {
        if (++wfc->counter == 100) {
            wfc->states[wfc->len] = wfc_clone(wfc->states[wfc->len - 1]);
            ++wfc->len;

            wfc->counter = 0;
        }
    }

    return 0;
}

int wfcBacktrack(struct WfcWrapper *wfc) {
    if (wfc->len <= 1) return -1;

    wfc_free(wfc->states[wfc->len - 1]);
    --wfc->len;
    wfc->counter = 0;

    return 0;
}

void wfcBlit(
    const struct WfcWrapper wfc,
    const unsigned char *src, unsigned char *dst) {
    assert(wfc.len > 0);

    int code = wfc_blit(wfc.states[wfc.len - 1], src, dst);
    assert(code == 0);
}

void wfcBlitAveraged(
    const struct WfcWrapper wfc,
    const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    assert(wfc.len > 0);

    int bytesPerPixel = wfc.bytesPerPixel;
    const wfc_State *state = wfc.states[wfc.len - 1];
    int pattCnt = wfc_patternCount(state);

    for (int j = 0; j < dstH; ++j) {
        for (int i = 0; i < dstW; ++i) {
            for (int b = 0; b < bytesPerPixel; ++b) {
                int sum = 0, cnt = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    int avail = wfc_patternAvailable(state, p, i, j);
                    assert(avail >= 0);
                    if (avail) {
                        const unsigned char* px =
                            wfc_pixelToBlit(state, p, i, j, src);
                        assert(px != NULL);

                        sum += (int)px[b];
                        ++cnt;
                    }
                }

                if (cnt == 0) continue;

                unsigned char avg = (unsigned char)(sum / cnt);
                dst[j * dstW * bytesPerPixel + i * bytesPerPixel + b] = avg;
            }
        }
    }
}

void wfcBlitObserved(
    const struct WfcWrapper wfc,
    const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    assert(wfc.len > 0);

    int bytesPerPixel = wfc.bytesPerPixel;
    const wfc_State *state = wfc.states[wfc.len - 1];
    int pattCnt = wfc_patternCount(state);

    for (int j = 0; j < dstH; ++j) {
        for (int i = 0; i < dstW; ++i) {
            bool foundSingle = false;
            int patt;
            for (int p = 0; p < pattCnt; ++p) {
                int avail = wfc_patternAvailable(state, p, i, j);
                assert(avail >= 0);
                if (avail) {
                    if (foundSingle) {
                        foundSingle = false;
                        break;
                    }

                    foundSingle = true;
                    patt = p;
                }
            }

            if (!foundSingle) continue;

            const unsigned char* px = wfc_pixelToBlit(state, patt, i, j, src);
            assert(px != NULL);

            memcpy(&dst[j * dstW * bytesPerPixel + i * bytesPerPixel], px,
                bytesPerPixel);
        }
    }
}

void wfcFree(struct WfcWrapper wfc) {
    for (int i = 0; i < wfc.len; ++i) wfc_free(wfc.states[i]);
    if (wfc.states != NULL) free(wfc.states);
}

int writeOut(const struct Args *args, int bytesPerPixel, void *pixels) {
    assert(args != NULL);
    assert(args->pathOut != NULL);
    assert(pixels != NULL);

    bool writeFailed = false;

    enum ImageFormat fmt = getImageFormat(args->pathOut);
    if (fmt == IMG_BMP) {
        if (stbi_write_bmp(args->pathOut,
                args->dstW, args->dstH, bytesPerPixel, pixels) == 0) {
            writeFailed = true;
        }
    } else if (fmt == IMG_PNG) {
        if (stbi_write_png(args->pathOut,
                args->dstW, args->dstH, bytesPerPixel,
                pixels,
                args->dstW * bytesPerPixel) == 0) {
            writeFailed = true;
        }
    } else if (fmt == IMG_TGA) {
        if (stbi_write_tga(args->pathOut,
                args->dstW, args->dstH, bytesPerPixel,
                pixels) == 0) {
            writeFailed = true;
        }
    } else {
        assert(false);
    }

    if (writeFailed) {
        fprintf(stderr, "Error writing to %s.\n", args->pathOut);
        return -1;
    }

    return 0;
}
