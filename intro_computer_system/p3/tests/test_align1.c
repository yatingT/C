// first pointer returned is 8-byte aligned
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "myHeap.h"

int main() {
   assert(myInit(4096) == 0);
   dispMem();
   int* ptr = (int*) myAlloc(sizeof(int));
   dispMem();
   assert(ptr != NULL);
   assert(((int)ptr) % 8 == 0);
   exit(0);
}
