/*
     This file is part of GNUnet.
     Copyright (C) 2001-2015 GNUnet e.V.

     GNUnet is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     GNUnet is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with GNUnet; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @author Christian Grothoff
 * @author Nils Durner
 *
 * @file
 * Container classes for GNUnet
 *
 * @defgroup dll  Container library: Doubly-linked list
 *
 */

#ifdef __cplusplus
extern "C"
{
#if 0                           /* keep Emacsens' auto-indent happy */
}
#endif
#endif

/* **************** circular doubly-linked list *************** */
/* To avoid mistakes: head->prev == taili && tail->next == head */

/**
 * @ingroup dll
 * Insert an element into a CDLL before the given other element.
 *
 * @param other prior element, NULL for insertion at head of DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_insert_before(other,element) do { \
    (element)->prev = (other)->prev; \
    (element)->next = other; \
    (other)->prev->next = element; \
    (other)->prev = element; \
  } while (0)


/**
 * @ingroup dll
 * Insert an element into a CDLL after the given other element.
 *
 * @param other prior element, NULL for insertion at head of DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_insert_after(other,element) do { \
    GNUNET_CONTAINER_CDLL_insert_before((other)->next,element); \
  } while (0)


/**
 * @ingroup dll
 * Insert an element at the tail of a CDLL. Assumes that head and
 * element are structs with prev and next fields.
 *
 * This can also be used to create a CDLL.
 *
 * @deprecated See #GNUNET_CONTAINER_CDLL_push(head,element)
 *
 * @param[in,out] head pointer to the head of the DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_insert_tail(head,element) do { \
  if ((head) == NULL) { \
    (element)->prev = element; \
    (element)->next = element; \
    (head) = element; \
  } else \
    GNUNET_CONTAINER_CDLL_insert_before(head,element); \
  } while (0)


/**
 * @ingroup dll
 * Insert an element at the tail of a CDLL. Assumes that head and
 * element are structs with prev and next fields.
 *
 * This can also be used to create a CDLL.
 *
 * @param[in,out] head pointer to the head of the DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_push(head,element) do { \
  GNUNET_CONTAINER_CDLL_insert_tail(head,element); \
  } while (0)


/**
 * @ingroup dll
 * Insert an element at the head of a CDLL. Assumes that head and
 * element are structs with prev and next fields.
 *
 * This can also be used to create a CDLL.
 *
 * @deprecated See #GNUNET_CONTAINER_CDLL_unshift(head,element)
 *
 * @param[in,out] head pointer to the head of the DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_insert(head,element) do { \
  GNUNET_CONTAINER_CDLL_insert_tail(head,element); \
  (head) = element; } while (0)


/**
 * @ingroup dll
 * Insert an element at the head of a CDLL. Assumes that head and
 * element are structs with prev and next fields.
 *
 * This can also be used to create a CDLL.
 *
 * @param[in,out] head pointer to the head of the DLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_unshift(head,element) do { \
  GNUNET_CONTAINER_CDLL_insert(head,element); \
  } while (0)


/**
 * @ingroup dll
 * Remove an element from a CDLL. Assumes that head and
 * element point to structs with prev and next fields.
 *
 * @param[in,out] head pointer to the head of the DLL
 * @param element element to remove
 */
#define GNUNET_CONTAINER_CDLL_remove(head,element) do { \
  if ((element) == (head)) { \
    if ((element)->next == (element)) { \
      (head) = NULL; \
    } else \
      (head) = (element)->next; \
  }; \
  (element)->prev->next = (element)->next; \
  (element)->next->prev = (element)->prev; \
} while (0)


/**
 * @ingroup dll
 * Insertion sort of @a element into CDLL sorted by @a comparator.
 *
 * @param TYPE element type of the elements, i.e. `struct ListElement`
 * @param comparator function like memcmp() to compare elements; takes
 *                   three arguments, the @a comparator_cls and two elements,
 *                   returns an `int` (-1, 0 or 1)
 * @param comparator_cls closure for @a comparator
 * @param[in,out] head head of CDLL
 * @param element element to insert
 */
#define GNUNET_CONTAINER_CDLL_insert_sorted(TYPE,comparator,comparator_cls,head,element) do { \
  if ( (NULL == head) || \
       (0 < comparator (comparator_cls, \
                        element, \
                        head)) ) \
  { \
    /* insert at head, element < head */ \
    GNUNET_CONTAINER_CDLL_unshift (head,   \
                                 element); \
  }   \
  else \
  {           \
    TYPE *pos; \
    \
    for (pos = head; \
         (head)->prev != pos; \
         pos = pos->next) \
      if (0 < \
          comparator (comparator_cls, \
                      element, \
                      pos)) \
        break; /* element < pos */ \
    if ((head)->prev == pos) /* => element > tail */ \
    { \
      if (0 < \
          comparator (comparator_cls, \
                      element, \
                      pos)) { \
        GNUNET_CONTAINER_CDLL_insert_before (pos,     \
                                             element); \
      } else \
        GNUNET_CONTAINER_CDLL_push (head,    \
                                    element); \
    } \
    else /* prev < element < pos */ \
    { \
      GNUNET_CONTAINER_CDLL_insert_before (pos,     \
                                           element); \
    } \
  } \
} while (0)


#if 0                           /* keep Emacsens' auto-indent happy */
{
#endif
#ifdef __cplusplus
}
#endif

