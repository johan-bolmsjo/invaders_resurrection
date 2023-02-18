typedef struct _PanicCleanUp PanicCleanUp;

struct _PanicCleanUp {
    void (*handler)();
    PanicCleanUp* prev;
    PanicCleanUp* next;
};

void warning(char* format, ...);
void panic(char* format, ...);
void panic_register_cleanup(PanicCleanUp* p);
