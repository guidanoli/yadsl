#ifndef __AA_VECTOR_H__
#define __AA_VECTOR_H__

/* Implementation of resizable vector
   Version 0.0 */

#include <stddef.h>

typedef enum
{
	/* All went well */
	AA_VECTOR_RET_OK = 0,

	/* Could not allocate memory */
	AA_VECTOR_RET_MEMORY,
}
aa_VectorRetID;

typedef void aa_VectorHandle;

/* Create a vector

   Parameters:
     * initial_size - vector initial size

   Returns:
     * OK - *vector_ptr now points to the newly created vector
	 * MEMORY
*/
aa_VectorRetID aa_vector_create(
	size_t initial_size,
	aa_VectorHandle** vector_ptr);

/* Destroy a vector
   
*/
void aa_vector_destroy(aa_VectorHandle* vector);

#endif