struct WfcWrapper {
    int dstW, dstH;

    int len, cap;
    struct wfc_State **states;
    int counter;

    int bytesPerPixel;
};

int wfcInit(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, const unsigned char *dst,
    bool *keep,
    struct WfcWrapper *wfc) {
    wfc->dstW = dstW;
    wfc->dstH = dstH;

    wfc->len = 0;
    // @TODO Create a flag for backtrack capacity.
    wfc->cap = 10;
    wfc->states = malloc((size_t)wfc->cap * sizeof(*wfc->states));

    struct wfc_State *state = wfc_initEx(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH, dst, NULL, keep);
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

int wfcPatternCount(const struct WfcWrapper wfc) {
    int pattCnt = wfc_patternCount(wfc.states[wfc.len - 1]);
    assert(pattCnt >= 0);
    return pattCnt;
}

int wfcStatus(const struct WfcWrapper wfc) {
    return wfc_status(wfc.states[wfc.len - 1]);
}

int wfcStep(struct WfcWrapper *wfc) {
    int status = wfc_step(wfc->states[wfc->len - 1]);
    if (status != 0) return status;

    if (wfc->len < wfc->cap) {
        // @TODO Create a flag for step duration between checkpoints.
        if (++wfc->counter == 1000) {
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
    const struct WfcWrapper wfc, bool modifOnly,
    const unsigned char *src, unsigned char *dst) {
    assert(wfc.len > 0);

    int bytesPerPixel = wfc.bytesPerPixel;
    const wfc_State *state = wfc.states[wfc.len - 1];
    int pattCnt = wfc_patternCount(state);

    for (int j = 0; j < wfc.dstH; ++j) {
        for (int i = 0; i < wfc.dstW; ++i) {
            if (modifOnly && !wfc_modifiedAt(state, i, j)) continue;

            for (int b = 0; b < bytesPerPixel; ++b) {
                int sum = 0, cnt = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    int avail = wfc_patternPresentAt(state, p, i, j);
                    assert(avail >= 0);
                    if (avail) {
                        const unsigned char* px =
                            wfc_pixelToBlitAt(state, src, p, i, j);
                        assert(px != NULL);

                        sum += (int)px[b];
                        ++cnt;
                    }
                }

                if (cnt == 0) continue;

                unsigned char avg = (unsigned char)(sum / cnt);
                dst[j * wfc.dstW * bytesPerPixel + i * bytesPerPixel + b] = avg;
            }
        }
    }
}

bool wfcIsCollapsed(const struct WfcWrapper wfc, int x, int y, int *patt) {
    assert(wfc.len > 0);

    const wfc_State *state = wfc.states[wfc.len - 1];
    int pattCnt = wfc_patternCount(state);

    bool foundSingle = false;
    for (int p = 0; p < pattCnt; ++p) {
        int avail = wfc_patternPresentAt(state, p, x, y);
        assert(avail >= 0);
        if (avail) {
            if (foundSingle) {
                foundSingle = false;
                break;
            }

            foundSingle = true;
            if (patt != NULL) *patt = p;
        }
    }

    return foundSingle;
}

int wfcCollapsedCount(const struct WfcWrapper wfc) {
    assert(wfc.len > 0);

    const wfc_State *state = wfc.states[wfc.len - 1];
    return wfc_collapsedCount(state);
}

void wfcBlitCollapsed(
    const struct WfcWrapper wfc, const unsigned char *src, unsigned char *dst) {
    assert(wfc.len > 0);

    int bytesPerPixel = wfc.bytesPerPixel;
    const wfc_State *state = wfc.states[wfc.len - 1];

    for (int j = 0; j < wfc.dstH; ++j) {
        for (int i = 0; i < wfc.dstW; ++i) {
            int patt;
            if (!wfcIsCollapsed(wfc, i, j, &patt)) continue;

            const unsigned char* px = wfc_pixelToBlitAt(state, src, patt, i, j);
            assert(px != NULL);

            memcpy(&dst[j * wfc.dstW * bytesPerPixel + i * bytesPerPixel], px,
                bytesPerPixel);
        }
    }
}

void wfcSetWhichCollapsed(const struct WfcWrapper wfc, bool *dst) {
    for (int j = 0; j < wfc.dstH; ++j) {
        for (int i = 0; i < wfc.dstW; ++i) {
            dst[j * wfc.dstW + i] = wfcIsCollapsed(wfc, i, j, NULL);
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
