/**
 * \file
 * L4 kernel event tracing
 * \ingroup api_calls_fiasco
 */
/*
 * (c) 2008-2009 Adam Lackorzynski <adam@os.inf.tu-dresden.de>,
 *               Björn Döbel <doebel@os.inf.tu-dresden.de>,
 *               Torsten Frenzel <frenzel@os.inf.tu-dresden.de>
 *     2015      Adam Lackorzynski <adam@l4re.org>
 *     economic rights: Technische Universität Dresden (Germany)
 *
 * License: see LICENSE.spdx (in this directory or the directories above)
 */
/*****************************************************************************/
#ifndef __L4_KTRACE_H__
#define __L4_KTRACE_H__

#include <l4/sys/types.h>
#include <l4/sys/ktrace_events.h>

/**
 * \defgroup fiasco_trace_api Kernel Tracing
 * \ingroup api_calls_fiasco
 * Kernel tracing related functionality.
 * \attention This API is subject to change!
 *
 * This is a tracing facility for the Fiasco kernel trace buffer. Any call to
 * any function might be invalid. Do not rely on it in any real code.
 *
 * \includefile{l4/sys/ktrace.h}
 */

/**
 * Create new trace-buffer entry with describing \<text\>.
 * \ingroup fiasco_trace_api
 *
 * \param  text   Logging text
 * \return Pointer to trace-buffer entry
 */
L4_INLINE l4_umword_t
fiasco_tbuf_log(const char *text);

/**
 * Create new trace-buffer entry with describing \<text\> and three additional
 * values.
 * \ingroup fiasco_trace_api
 *
 * \param  text   Logging text
 * \param  v1     first value
 * \param  v2     second value
 * \param  v3     third value
 * \return Pointer to trace-buffer entry
 */
L4_INLINE l4_umword_t
fiasco_tbuf_log_3val(const char *text, l4_umword_t v1, l4_umword_t v2, l4_umword_t v3);

/**
 * Create new trace-buffer entry with binary data.
 * \ingroup fiasco_trace_api
 *
 * \param  data       binary data
 * \return Pointer to trace-buffer entry
 */
L4_INLINE l4_umword_t
fiasco_tbuf_log_binary(const unsigned char *data);

/**
 * Clear trace-buffer.
 * \ingroup fiasco_trace_api
 */
L4_INLINE void
fiasco_tbuf_clear(void);

/**
 * Dump trace-buffer to kernel console.
 * \ingroup fiasco_trace_api
 */
L4_INLINE void
fiasco_tbuf_dump(void);

#include <l4/sys/__ktrace-impl.h>

#endif
