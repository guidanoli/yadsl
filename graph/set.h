#ifndef __SET_H__
#define __SET_H__

/**
* A Set starts empty.
* You are able to add and remove numbers.
* A number can go from 0 to 2^32 - 1.
*/

typedef enum
{
    /* All went ok */
    SET_RETURN_OK,

    /* Invalid parameter was provided */
    SET_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    SET_RETURN_MEMORY,

    /* Set contains a number */
    SET_RETURN_CONTAINS,

    /* Set does not contain a number */
    SET_RETURN_DOES_NOT_CONTAIN,
}
SetReturnID;

typedef struct Set Set;

SetReturnID setCreate(Set **ppSet);
SetReturnID setContains(Set *pSet, size_t value);
SetReturnID setAdd(Set *pSet, size_t value);
SetReturnID setRemove(Set *pSet, size_t value);
void setDestroy(Set *pSet);

#endif
