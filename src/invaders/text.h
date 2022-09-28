typedef struct _Text
{
  uint8_t *s;
  uint8_t c;
  int colour;
  int x_off;
  int x;
  int y;
  int offset;
} Text;

int
text_decode_font ();

void
text_print_char_adr  (uint8_t c, uint16_t colour, uint16_t *dst);

void
text_print_str_adr (uint8_t *s, uint16_t colour, uint16_t *dst);

void
text_print_str_fancy_init (Text *t, uint8_t *s, int x_off, int x, int y);

void
text_print_str_fancy (DG *di, Text *t);
