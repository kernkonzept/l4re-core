static __inline__ void __attribute__((always_inline))
init_one_static_tls (pthread_descr descr, struct link_map *map)
{
# if defined(TLS_TCB_AT_TP)
  dtv_t *dtv = GET_DTV (descr);
  void *dest = (char *) descr - map->l_tls_offset;
# elif defined(TLS_DTV_AT_TP)
  dtv_t *dtv = GET_DTV ((pthread_descr) ((char *) descr + TLS_PRE_TCB_SIZE));
  void *dest = (char *) descr + map->l_tls_offset + TLS_PRE_TCB_SIZE;
# else
#  error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined"
# endif

  /* Fill in the DTV slot so that a later LD/GD access will find it.  */
  dtv[map->l_tls_modid].pointer.val = dest;
  dtv[map->l_tls_modid].pointer.is_static = true;

  /* Initialize the memory.  */
  memset (mempcpy (dest, map->l_tls_initimage, map->l_tls_initimage_size),
    '\0', map->l_tls_blocksize - map->l_tls_initimage_size);
}

static void
__pthread_init_static_tls (struct link_map *map)
{
  pthread_descr th;

  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive)
    {
      init_one_static_tls(th, map);
    }
}
