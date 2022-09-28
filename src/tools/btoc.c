/* Convert binary data to character array */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define BUF_SIZE  4096


int
main (int argc, char **argv)
{
  int fd, len, i, j, r, left, written = 0;
  unsigned char buf[BUF_SIZE], *b;
  
  if (argc != 3)
    {
      fprintf (stderr, "usage: btoc <binary file> <name>\n");
      return 1;
    }
  
  fd = open (argv[1], O_RDONLY);
  if (fd == -1)
    {
      fprintf (stderr, "Could not open input file!\n");
      return 1;
    }
  
  len = lseek (fd, 0, SEEK_END);
  lseek (fd, 0, SEEK_SET);
  
  printf ("#define %s_length  %d\n\n", argv[2], len);
  printf ("static unsigned char %s_data[%d] =\n{\n", argv[2], len);
  
  while ((r = read (fd, buf, BUF_SIZE)) > 0)
    {
      for (i = 0; i < r; )
	{
	  b = buf + i;
	  if (written < (len - 8))
	    {
	      printf ("  0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, "
		      "0x%02x, 0x%02x,\n",
		      b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	      written += 8;
	      i += 8;
	    }
	  else
	    {
	      left = len - written;
	      printf ("  ");
	      for (j = 0; j < left; j++)
		{
		  printf ("0x%02x", b[j]);
		  if (j < (left - 1))
		    printf (", ");
		}
	      
	      printf ("\n");
	      
	      written += left;
	      i += left;
	    }
	}
    }
  
  printf ("};\n");
  
  return 0;
}
