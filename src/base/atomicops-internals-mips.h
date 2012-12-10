/* Copyright (c) 2011, Google Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ---
 * Authors: Ray Balogh   <ray.balogh@sycamorenet.com>
 *          Aaron Conole <apconole@yahoo.com>
 */

// Implementation of atomic operations for mips-linux.  This file should not
// be included directly.  Clients should instead include
// "base/atomicops.h".

#ifndef BASE_ATOMICOPS_INTERNALS_MIPS_H_
#define BASE_ATOMICOPS_INTERNALS_MIPS_H_

typedef int32_t Atomic32;

#ifdef __MIPS64__
#define BASE_HAS_ATOMIC64 1
#endif

namespace base {
namespace subtle {

static inline void _sync(void) {
  __asm__ __volatile__("sync; nop": : : "memory");
}

static inline Atomic32 OSAtomicAdd32(Atomic32 amount, Atomic32 *value)
{
    Atomic32 temp, result;

    __asm__ __volatile__(
        ".set push\n"
        ".set noreorder\n"
        "1:    ll      %1, %4\n"	// temp <-[link] *value
        "      addu    %0, %1, %3\n"	// result <- temp + amount
        "      sc      %0, %2\n"	// *value <-[cond] result
        "      beqz    %0, 1b\n"	// if ![cond] goto 1
        "      addu    %0, %1, %3\n"	// result <- temp + amount
        ".set pop\n"
        : "=&r" (result), "=&r" (temp), "=m" (*value)
        : "Ir" (amount), "m" (*value)
        : "memory");

    return result;
}

static inline Atomic32 OSAtomicAdd32Barrier(Atomic32 amount, Atomic32 *value)
{
  Atomic32 t;
  _sync();
  t = OSAtomicAdd32(amount, value);
  _sync();
  return t;
}

static inline bool OSAtomicCompareAndSwap32(Atomic32 old_value,
                                            Atomic32 new_value,
                                            Atomic32 *value)
{
    Atomic32 prev;

    __asm__ __volatile__(
        ".set push\n"
        ".set noreorder\n"
	"1:    ll      %0, %2\n"	// prev <-[link] *value
	"      bne     %0, %3, 2f\n"	// if prev != old_value
	"      move    %0, $0\n"	//     prev <- 0 ; goto 2
	"      move    %0, %4\n"	// prev <- new_value
	"      sc      %0, %1\n"	// *value <-[cond] prev
	"      beqz    %0, 1b\n"	// if ![prev <- cond] goto 1
	"      nop\n"			// (delay slot)
	"2:\n"
        ".set pop\n"
        : "=&r" (prev), "=m" (*value)
        : "m" (*value), "r" (old_value), "r" (new_value)
        : "memory");

  return prev;
}

static inline Atomic32 OSAtomicCompareAndSwap32Acquire(Atomic32 old_value,
                                                       Atomic32 new_value,
                                                       Atomic32 *value)
{
  Atomic32 t = OSAtomicCompareAndSwap32(old_value, new_value, value);
  _sync();
  return t;
}

static inline Atomic32 OSAtomicCompareAndSwap32Release(Atomic32 old_value,
                                                       Atomic32 new_value,
                                                       Atomic32 *value)
{
  _sync();
  return OSAtomicCompareAndSwap32(old_value, new_value, value);
}

inline void MemoryBarrier()
{
  _sync();
}

// 32-bit Versions.

inline Atomic32 NoBarrier_CompareAndSwap(volatile Atomic32 *ptr,
                                         Atomic32 old_value,
                                         Atomic32 new_value)
{
  Atomic32 prev_value;
  do {
    if (OSAtomicCompareAndSwap32(old_value, new_value,
                                 const_cast<Atomic32*>(ptr))) {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

inline Atomic32 NoBarrier_AtomicExchange(volatile Atomic32 *ptr,
                                         Atomic32 new_value)
{
  Atomic32 old_value;
  do {
    old_value = *ptr;
  } while (!OSAtomicCompareAndSwap32(old_value, new_value,
                                     const_cast<Atomic32*>(ptr)));
  return old_value;
}

inline Atomic32 NoBarrier_AtomicIncrement(volatile Atomic32 *ptr,
                                          Atomic32 increment)
{
  return OSAtomicAdd32(increment, const_cast<Atomic32*>(ptr));
}

inline Atomic32 Barrier_AtomicIncrement(volatile Atomic32 *ptr,
                                        Atomic32 increment)
{
  return OSAtomicAdd32Barrier(increment, const_cast<Atomic32*>(ptr));
}

inline Atomic32 Acquire_CompareAndSwap(volatile Atomic32 *ptr,
                                       Atomic32 old_value,
                                       Atomic32 new_value)
{
  Atomic32 prev_value;
  do
  {
    if (OSAtomicCompareAndSwap32Acquire(old_value, new_value,
                                        const_cast<Atomic32*>(ptr)))
    {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

inline Atomic32 Release_CompareAndSwap(volatile Atomic32 *ptr,
                                       Atomic32 old_value,
                                       Atomic32 new_value)
{
  Atomic32 prev_value;
  do
  {
    if (OSAtomicCompareAndSwap32Release(old_value, new_value,
                                        const_cast<Atomic32*>(ptr)))
    {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

#ifdef __MIPS64__

typedef int64_t Atomic64;

// 64-bit Versions.

static inline Atomic64 OSAtomicAdd64(Atomic64 amount, Atomic64 *value)
{
    Atomic64 temp, result;

    __asm__ __volatile__(
        ".set push\n"
        ".set noreorder\n"
        "1:    lld     %1, %4\n"	// temp <-[link] *value
        "      daddu   %0, %1, %3\n"	// result <- temp + amount
        "      scd     %0, %2\n"	// *value <-[cond] result
        "      beqz    %0, 1b\n"	// if ![cond] goto 1
        "      daddu   %0, %1, %3\n"	// result <- temp + amount
        ".set pop\n"
        : "=&r" (result), "=&r" (temp), "=m" (*value)
        : "Ir" (amount), "m" (*value)
        : "memory");

    return result;
}

static inline Atomic64 OSAtomicAdd64Barrier(Atomic64 amount, Atomic64 *value)
{
  Atomic64 t;
  _sync();
  t = OSAtomicAdd64(amount, value);
  _sync();
  return t;
}

static inline bool OSAtomicCompareAndSwap64(Atomic64 old_value,
                                            Atomic64 new_value,
                                            Atomic64 *value)
{
    Atomic64 prev;

    __asm__ __volatile__(
        ".set push\n"
        ".set noreorder\n"
	"1:    lld     %0, %2\n"	// prev <-[link] *value
	"      bne     %0, %3, 2f\n"	// if prev != old_value
	"      move    %0, $0\n"	//     prev <- 0 ; goto 2
	"      move    %0, %4\n"	// prev <- new_value
	"      scd     %0, %1\n"	// *value <-[cond] prev
	"      beqz    %0, 1b\n"	// if ![prev <- cond] goto 1
	"      nop\n"			// (delay slot)
	"2:\n"
        ".set pop\n"
        : "=&r" (prev), "=m" (*value)
        : "m" (*value), "r" (old_value), "r" (new_value)
        : "memory");

  return prev;
}

static inline Atomic64 OSAtomicCompareAndSwap64Acquire(Atomic64 old_value,
                                                       Atomic64 new_value,
                                                       Atomic64 *value)
{
  Atomic64 t = OSAtomicCompareAndSwap64(old_value, new_value, value);
  _sync();
  return t;
}

static inline Atomic64 OSAtomicCompareAndSwap64Release(Atomic64 old_value,
                                                       Atomic64 new_value,
                                                       Atomic64 *value)
{
  _sync();
  return OSAtomicCompareAndSwap64(old_value, new_value, value);
}


inline Atomic64 NoBarrier_CompareAndSwap(volatile Atomic64 *ptr,
                                         Atomic64 old_value,
                                         Atomic64 new_value)
{
  Atomic64 prev_value;
  do
  {
    if (OSAtomicCompareAndSwap64(old_value, new_value,
                                 const_cast<Atomic64*>(ptr)))
    {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

inline Atomic64 NoBarrier_AtomicExchange(volatile Atomic64 *ptr,
                                         Atomic64 new_value)
{
  Atomic64 old_value;
  do
  {
    old_value = *ptr;
  } while (!OSAtomicCompareAndSwap64(old_value, new_value,
                                     const_cast<Atomic64*>(ptr)));
  return old_value;
}

inline Atomic64 NoBarrier_AtomicIncrement(volatile Atomic64 *ptr,
                                          Atomic64 increment)
{
  return OSAtomicAdd64(increment, const_cast<Atomic64*>(ptr));
}

inline Atomic64 Barrier_AtomicIncrement(volatile Atomic64 *ptr,
                                        Atomic64 increment)
{
  return OSAtomicAdd64Barrier(increment, const_cast<Atomic64*>(ptr));
}

inline Atomic64 Acquire_CompareAndSwap(volatile Atomic64 *ptr,
                                       Atomic64 old_value,
                                       Atomic64 new_value)
{
  Atomic64 prev_value;
  do
  {
    if (OSAtomicCompareAndSwap64Acquire(old_value, new_value,
                                        const_cast<Atomic64*>(ptr)))
    {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

inline Atomic64 Release_CompareAndSwap(volatile Atomic64 *ptr,
                                       Atomic64 old_value,
                                       Atomic64 new_value)
{
  Atomic64 prev_value;
  do
  {
    if (OSAtomicCompareAndSwap64Release(old_value, new_value,
                                        const_cast<Atomic64*>(ptr)))
    {
      return old_value;
    }
    prev_value = *ptr;
  } while (prev_value == old_value);
  return prev_value;
}

#endif

inline void NoBarrier_Store(volatile Atomic32 *ptr, Atomic32 value)
{
  *ptr = value;
}

inline void Acquire_Store(volatile Atomic32 *ptr, Atomic32 value)
{
  *ptr = value;
  _sync();
}

inline void Release_Store(volatile Atomic32 *ptr, Atomic32 value)
{
  _sync();
  *ptr = value;
}

inline Atomic32 NoBarrier_Load(volatile const Atomic32 *ptr)
{
  return *ptr;
}

inline Atomic32 Acquire_Load(volatile const Atomic32 *ptr)
{
  Atomic32 value = *ptr;
  _sync();
  return value;
}

inline Atomic32 Release_Load(volatile const Atomic32 *ptr)
{
  _sync();
  return *ptr;
}

#ifdef __MIPS64__

// 64-bit Versions.

inline void NoBarrier_Store(volatile Atomic64 *ptr, Atomic64 value)
{
  *ptr = value;
}

inline void Acquire_Store(volatile Atomic64 *ptr, Atomic64 value)
{
  *ptr = value;
  _sync();
}

inline void Release_Store(volatile Atomic64 *ptr, Atomic64 value)
{
  _sync();
  *ptr = value;
}

inline Atomic64 NoBarrier_Load(volatile const Atomic64 *ptr)
{
  return *ptr;
}

inline Atomic64 Acquire_Load(volatile const Atomic64 *ptr)
{
  Atomic64 value = *ptr;
  _sync();
  return value;
}

inline Atomic64 Release_Load(volatile const Atomic64 *ptr)
{
  _sync();
  return *ptr;
}

#endif

}   // namespace base::subtle
}   // namespace base

#endif  // BASE_ATOMICOPS_INTERNALS_MIPS_H_
