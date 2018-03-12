/**
 * \file
 * \brief List implemenation
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Ronald Aigner <ra3@os.inf.tu-dresden.de>
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#ifndef __L4UTIL__SLL_H__
#define __L4UTIL__SLL_H__

#include <stdlib.h>

#include <l4/sys/compiler.h>

EXTERN_C_BEGIN

/*
 * the linked list structure
 */

typedef struct slist_t
{
  struct slist_t *next;   /* pointer to next node */
  void *data;          /* void pointer for user data */
} slist_t;

/*
 * function prototypes
 */
static inline slist_t*
list_new_entry(void *data);

static inline slist_t*
list_append(slist_t *list, slist_t *new_node);

static inline slist_t*
list_remove(slist_t *list, slist_t *node);

static inline void
list_free_entry(slist_t **list);

static inline unsigned char
list_is_empty(slist_t *list);

static inline slist_t*
list_get_at(slist_t *list, int n);

static inline slist_t*
list_add(slist_t *list, slist_t *new_node);

static inline void
list_insert_after(slist_t *after, slist_t *new_node);

static inline int
list_elements(slist_t *head);

/*
 *  allocateNode()
 *  allocate a new node.
 *
 *  Parameters:
 *  void    *data       a generic pointer to object data
 *
 *  Return Values:
 *  pointer to slist_t if succeeds
 *  NULL otherwise
 *
 */
static inline slist_t*
list_new_entry(void *data)
{
  slist_t *sll;

  sll = (slist_t *)malloc(sizeof(slist_t));
  if (!sll)
    return ((slist_t *) NULL);

  sll->data=data;
  sll->next=NULL;

  return (sll);
}

/*
 *  appendNode()
 *  appends a node to the end of a list
 *
 *  Parameters:
 *  slist_t *head      - modify the list
 *  slist_t *new       - appends this node
 *
 *  Return Values:
 *  the new list
 *
 */
static inline slist_t*
list_append(slist_t *head, slist_t *new_node)
{
  slist_t *ret = head;
  if (!head)
    return new_node;

  while (head->next)
    head = head->next;
  head->next = new_node;
  return ret;
}

/*
 *  insertNode()
 *  insert a node at the beginning of a list
 *
 *  Parameters:
 *  slist_t *head      - modify this list
 *  slist_t *new       - appends this node
 *
 *  Return Values:
 *  the new list
 *
 */
static inline slist_t*
list_add(slist_t *head, slist_t *new_node)
{
  if (!new_node)
    return head;
  new_node->next = head;
  return new_node;
}

/*
 *  insertNode()
 *  insert a node at the beginning of a list
 *
 *  Parameters:
 *  slist_t *head      - modify this list
 *  slist_t *new       - appends this node
 *
 *  Return Values:
 *  the new list
 *
 */
static inline void
list_insert_after(slist_t *after, slist_t *new_node)
{
  if (!new_node)
    return;
  if (!after)
    return;
  new_node->next = after->next;
  after->next = new_node;
}


/*
 *  emptyList()
 *  check if a list variable is NULL
 *
 *  Parameters:
 *  slist_t *list      list
 *
 *  Return Values:
 *  TRUE    if empty
 *  FALSE   if not empty
 *
 */
static inline unsigned char
list_is_empty(slist_t *list)
{
  return ((list) ? 0 : 1);
}

/*
 *  delNode()
 *  remove a node from a list
 *
 *  Parameters:
 *  slist_t  *head      - list to modify
 *  slist_t *node       - node to remove
 *
 *  Return Values:
 *  none
 *
 */
static inline slist_t*
list_remove(slist_t *head, slist_t *node)
{
  slist_t *ret = head;
  if (list_is_empty(head))
    return ret;
  if (!node)
    return ret;

  if (head == node)
    {
      ret = head->next;
    }
  else
    {
      while (head && (head->next != node))
	head = head->next;
      if (!head)
	return ret;
      else
	head->next = node->next;
    }
  list_free_entry(&node);
  return ret;
}

/*
 *  freeNode()
 *  frees a node
 *
 *  Parameters:
 *  slist_t  *list  node to free
 *
 *  Return Values:
 *  none
 *
 */
static inline void
list_free_entry(slist_t **list)
{
  if (*list)
    {
      free ((void *) (*list));
      (*list)=NULL;
    }
}


/*
 *  getNthNode()
 *  get nth node in a list
 *
 *  Parameters:
 *  slist_t *list       - the head list
 *  int n           - return the node
 *  Return Values:
 *  a pointer to the list at position n
 *  NULL if there's no such node at posion n
 *
 */
static inline slist_t*
list_get_at(slist_t *list, int n)
{
  int j=0;

  while (list)
    {
      j++;
      if (j == n)
	return (list);
      list = list->next;
    }

  return ((slist_t *) NULL);
}

/*
 *  numNodes()
 *  returns number of nodes in the list
 *
 *  Parameters:
 *  slist_t  *head      - the head node of the list
 *
 *  Return Values:
 *  number of node/s
 *
 */
static inline int
list_elements(slist_t *head)
{
  register int n;
  for (n=0; head; head=head->next) n++;
  return (n);
}

EXTERN_C_END

#endif  /* __L4UTIL__SLL_H__ */
