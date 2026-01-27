#!/bin/bash

# -Werror tells compiler to treat all the warnings as errors
# -Wall includes all warnings - uninitialized/unused variables
# -Wextra for extra warnings - different types' comparison, unused functions' arguments, etc.
BUILD_DIR=$PWD
[ "$(basename $BUILD_DIR)" != "rpn-calculator" ] && exit
CFLAGS_DEBUG="-Werror -Wall -Wextra -DDEBUG -g"
CFLAGS_RELEASE="-Werror -Wall -Wextra"
[ "$1" == "-release" ] && \
  CFLAGS=$CFLAGS_RELEASE || \
  CFLAGS=$CFLAGS_DEBUG
for item in $BUILD_DIR/*.c; do
  gcc -c $CFLAGS $item
done
mkdir -p bin && gcc rpn.o rpn_utils.o -lm -o bin/rpn
