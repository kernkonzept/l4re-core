/*!
 * \file   l4util/lib/src/alloc.c
 * \brief  allocator using a bit-array
 *
 * \date   08/25/2004
 * \author Jork Loeser <jork.loeser@inf.tu-dresden.de>
 *
 */
/*
 * (c) 2004-2009 Author(s)
 *     economic rights: Technische Universität Dresden (Germany)
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <stdlib.h>
#include <string.h>
#include <l4/util/alloc.h>


/*!\brief Initialize allocator array
 */
L4_CV l4util_alloc_t *l4util_alloc_init(int count, int base){
    l4util_alloc_t *alloc;
    if((alloc=malloc(sizeof(l4util_alloc_t)))==0) return 0;
    if((alloc->bits = malloc((count+(L4UTIL_ALLOC_BITS_SIZE-1))/8))==0){
	free(alloc);
	return 0;
    }
    memset(alloc->bits, 0, (count+(L4UTIL_ALLOC_BITS_SIZE-1))/8);
    alloc->count = count;
    alloc->base=base;
    alloc->next_elem = base;
    return alloc;
}

/*!\brief Return if an element is avail
 *
 * \retval 0	not avail
 * \retval 1	avail
 */
L4_CV int l4util_alloc_avail(l4util_alloc_t *alloc, int elem){
    if(elem<alloc->base || elem >= alloc->base+alloc->count) return 0;
    elem-=alloc->base;
    return !l4util_test_bit(elem&(L4UTIL_ALLOC_BITS_SIZE-1),
	                    alloc->bits+(elem/L4UTIL_ALLOC_BITS_SIZE));
}

/*!\brief Set an element busy
 *
 * \param  elem		element
 * \retval 0		OK
 * \retval 1		was occupied already or out of bound
 */
L4_CV int l4util_alloc_occupy(l4util_alloc_t *alloc, int elem){
    if(elem<alloc->base || elem >= alloc->base+alloc->count) return 1;
    elem-=alloc->base;
    return l4util_test_and_set_bit(elem&(L4UTIL_ALLOC_BITS_SIZE-1),
				   alloc->bits+(elem/L4UTIL_ALLOC_BITS_SIZE))?1:0;
}

/*!\brief Allocate any element
 *
 * \retval >0		element
 * \retval -1		Error, none free
 */
L4_CV int l4util_alloc_alloc(l4util_alloc_t *alloc){
    int elem=alloc->next_elem;

    while(l4util_alloc_occupy(alloc, elem)){
	if(++elem >= alloc->base + alloc->count) elem=0;
	if(elem==alloc->next_elem) return -1;
    }
    alloc->next_elem = elem + 1;
    if(alloc->next_elem == alloc->base + alloc->count){
	alloc->next_elem = alloc->base;
    }
    return elem;
}

/*!\brief Free an element
 * \param  elem		element to free
 * \retval 0		OK
 * \retval 1		was not occupied or out of bound
 */
L4_CV int l4util_alloc_free(l4util_alloc_t *alloc, int elem){
    if(elem < alloc->base || elem >= alloc->base+alloc->count){
	return 1;
    }
    elem-=alloc->base;
    return l4util_test_and_clear_bit(elem&(L4UTIL_ALLOC_BITS_SIZE-1),
				     alloc->bits+(elem/L4UTIL_ALLOC_BITS_SIZE))?0:1;
}
