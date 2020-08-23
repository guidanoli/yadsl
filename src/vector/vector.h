#ifndef __YADSL_VECTOR_H__
#define __YADSL_VECTOR_H__

/* Implementation of resizable vector
   Version 0.0 */

#include <stddef.h>

typedef enum
{
	/* All went well */
	yadsl_VECTOR_RET_OK = 0,

	/* Could not allocate memory */
	yadsl_VECTOR_RET_MEMORY,
}
yadsl_VectorRetID;

typedef void yadsl_VectorHandle;

/* Create a vector

   Parameters:
     * initial_size - vector initial size

   Returns:
     * OK - *vector_ptr now points to the newly created vector
	 * MEMORY
*/
yadsl_VectorRetID yadsl_vector_create(
	size_t initial_size,
	yadsl_VectorHandle** vector_ptr);

/* Destroy a vector
   
*/
void yadsl_vector_destroy(yadsl_VectorHandle* vector);

#endif
