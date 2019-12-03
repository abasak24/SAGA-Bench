/* Emulation of Full Empty Bits Using atomic check-and-swap.
 *
 * NOTES:
 * - Using these functions means that the MARKER value defined
 *   below must be reserved in your application and CANNOT be
 *   considered a normal value.  Feel free to change the value to
 *   suit your application.
 * - Improper use of these functions can and will result in deadlock.
 *
 * author: rmccoll3@gatech.edu
 */

#include  "x86_full_empty.h"
#include <stdlib.h>
#include <stdint.h>

int64_t
readfe(volatile int64_t * v) {
  stinger_memory_barrier();
  int64_t val;
  while(1) {
    val = *v;
    while(val == MARKER) {
      val = *v;
    }
    if(val == stinger_int64_cas(v, val, MARKER))
      break;
  }
  return val;
}

int64_t
writeef(volatile int64_t * v, int64_t new_val) {
  stinger_memory_barrier();
  int64_t val;
  while(1) {
    val = *v;
    while(val != MARKER) {
      val = *v;
    }
    if(MARKER == stinger_int64_cas(v, MARKER, new_val))
      break;
  }
  return val;
}

int64_t
readff(volatile int64_t * v) {
  stinger_memory_barrier();
  int64_t val = *v;
  while(val == MARKER) {
    val = *v;
  }
  return val;
}

int64_t
writeff(volatile int64_t * v, int64_t new_val) {
  stinger_memory_barrier();
  int64_t val;
  while(1) {
    val = *v;
    while(val == MARKER) {
      val = *v;
    }
    if(val == stinger_int64_cas(v, val, new_val))
      break;
  }
  return val;
}

int64_t
writexf(volatile int64_t * v, int64_t new_val) {
  stinger_memory_barrier();
  *v = new_val;
  stinger_memory_barrier();
  return new_val;
}

// ---------------------------------------------------

bool
writexf_bool(volatile bool * v, bool new_val) {
  stinger_memory_barrier();
  *v = new_val;
  stinger_memory_barrier();
  return new_val;
}

bool
readfe_bool(volatile bool * v) {
  stinger_memory_barrier();
  bool val;
  while(1) {
    val = *v;
    while(val == 0) {
      val = *v;
    }
    if(val == __sync_val_compare_and_swap(v, val, 0))
      break;
  }
  return val;
}
