typedef struct _FileFormat FileFormat;

struct _FileFormat {
    char** exts;
    Image* (*func)(char*);
};

Image* ff_read(char* path);
