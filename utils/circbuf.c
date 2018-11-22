/* Circular buffer example, keeps one slot open */
#include <stdlib.h>
#include <stdio.h>
#include "circbuf.h"




/* ������������� ������, ���� ��� ���!  */
int cb_init(CircularBuffer * cb, int size)
{
    cb->size = size + 1;	/* include empty elem */
    cb->start = 0;
    cb->end = 0;

    if (cb->elems == NULL) {
	cb->elems = (ElemType *) MALLOC(cb->size * sizeof(ElemType));

	if (cb->elems == NULL)	/* �� ���� �������� ������ */
	    return -1;
    }
   return 0;
}


/* ������� �����, ���� �� ����������! */
void cb_free(CircularBuffer * cb)
{
    if (cb->elems != NULL) {
	FREE(cb->elems);	/* OK if null */
	cb->elems = 0;
    }
}

/**
 * ����� ������?
 */
int cb_is_full(CircularBuffer * cb)
{
    return (cb->end + 1) % cb->size == cb->start;
}

/**
 * ����� ������?
 */
int cb_is_empty(CircularBuffer * cb)
{
    return cb->end == cb->start;
}

/**
 * �������� �����
 */
void cb_clear(CircularBuffer * cb)
{
    cb->start = 0;
    cb->end = 0;    
}


/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
void cb_write(CircularBuffer * cb, ElemType * elem)
{
    cb->elems[cb->end] = *elem;
    cb->end = (cb->end + 1) % cb->size;
    if (cb->end == cb->start)
	cb->start = (cb->start + 1) % cb->size;	/* full, overwrite */
}

/* Read oldest element. App must ensure !cbIsEmpty() first. */
void cb_read(CircularBuffer * cb, ElemType * elem)
{
    *elem = cb->elems[cb->start];
     cb->start = (cb->start + 1) % cb->size;
}
