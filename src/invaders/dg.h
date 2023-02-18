/* Before NEO a simple framebuffer hack was used.
 * This is the hack made to use NEO instead.
 */

#define DG_XRES   640  /* Screen width in pixels */
#define DG_YRES   480  /* Screen height in pixels */
#define DG_VFREQ   60  /* Vertical frequency */
#define DG_BITS    16  /* Bits per pixel */
#define DG_LLEN  1280  /* Line lenght in bytes */

/* Direct graphics information.
 */

typedef struct _DG {
    int vfreq; /* Vertical frequency */
    int vis;
    int hid;
    uint16_t* adr[2]; /* Two screens for page flipping */
} DG;

/* Flip screens
 */

void dg_flip(DG* dg);
