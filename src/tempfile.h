/* create and delete temporary files */

#include <stdio.h>

#ifdef _MSC_VER
#  define TEMP_FILENAME_SIZE L_tmpnam_s
#else
#  define TEMP_FILENAME_SIZE 25
#endif

/* Create and open a new temporary file
    @param filename a butter of size TEMP_FILENAME_SIZE where the filename will be stored.
    @return a handle to the open file. The temporary file's name will be stored in the given buffer.
     Returns NULL on error. */
FILE *open_temporary_file(char *filename);
