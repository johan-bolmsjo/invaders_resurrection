#pragma once

// TODO(jb): Review use of these defines. Convert to enum if the need
// persist. Seems like they belong in error.h rather than here!

/* Error codes
 */

#define E_OK     0
#define ERROR   -1 /* Generic fault */
#define E_OPEN  -2 /* Open fault */
#define E_CLOSE -3 /* Open fault */
#define E_SEEK  -4 /* Seek fault */
#define E_WRITE -5 /* Write fault */
#define E_READ  -6 /* Read fault */
#define E_MEM   -7 /* Out of memory */
