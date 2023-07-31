bool strEqIgnoreCase(const char *s1, const char *s2) {
    while (*s1 != 0 && s2 != 0) {
        if (tolower(*s1) != tolower(*s2)) return false;

        ++s1;
        ++s2;
    }
    return *s1 == 0 && *s2 == 0;
}

enum ImageFormat {
    IMG_INVALID,
    IMG_BMP,
    IMG_PNG,
    IMG_TGA,
};

enum ImageFormat getImageFormat(const char *file) {
    size_t len = strlen(file);
    if (len < 4) return IMG_INVALID;

    const char *ext = file + len - 4;

    if (strEqIgnoreCase(ext, ".bmp")) return IMG_BMP;
    else if (strEqIgnoreCase(ext, ".png")) return IMG_PNG;
    else if (strEqIgnoreCase(ext, ".tga")) return IMG_TGA;
    return IMG_INVALID;
}
