#include <stddef.h>
#include "connmgr.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
  connmgr_listen(5678);
  return 0;
}