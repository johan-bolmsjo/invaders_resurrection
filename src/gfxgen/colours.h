void
colour_bgr_to_rgb (Image *image);

Image *
colour_rgb_to_rgba (Image *image);

Image *
colour_rgba_to_rgb (Image *image);

void
colour_make_rgb_transparent (Image *image, uint8_t *rgb, uint8_t *thres);
