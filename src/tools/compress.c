/* Compress data with gzip deflate algorithm.
   Output is written to stdout. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <zlib.h>

int
main(int argc, char** argv)
{
    int fd;
    Bytef *src_buf, *dst_buf;
    uLong src_len, dst_len;

    if (argc != 2) {
        fprintf(stderr, "usage: compress <file>\n");
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to open file!\n");
        return 1;
    }

    src_len = lseek(fd, 0, SEEK_END);
    dst_len = src_len + (src_len >> 9) + 12;
    lseek(fd, 0, SEEK_SET);

    if (!(src_buf = malloc(src_len)))
        return 1;

    if (!(dst_buf = malloc(dst_len)))
        return 1;

    read(fd, src_buf, src_len);

    compress2(dst_buf, &dst_len, src_buf, src_len, 9);

    src_len = htonl(src_len);
    dst_len = htonl(dst_len);

    write(1, &src_len, 4);
    write(1, &dst_len, 4);

    dst_len = ntohl(dst_len);
    write(1, dst_buf, dst_len);

    free(src_buf);
    free(dst_buf);

    return 0;
}
