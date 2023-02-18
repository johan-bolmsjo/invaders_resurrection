#define DEF_MAX_CMD_SIZE 100

typedef struct _DefFile {
    char* mem;
    int size;
    int pos;
} DefFile;

typedef struct _DefObject {
    GfxObject* o; /* Object to attach frames to */
    int frames;
    Image** ipp;
    Clip clip;
    int x_off; /* Hot spot */
    int y_off;
} DefObject;

typedef struct _DefFunc {
    char* magic;
    int (*func)(DefObject*);
} DefFunc;

int def_run(char* path);
