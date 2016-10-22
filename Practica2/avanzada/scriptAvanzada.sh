#!/bin/bash

##A)COMPROBAMOS SI YA EXISTE TMP Y SI ES UN DIRECTORIO -> BORRAMOS
if test -e tmp && test -d tmp; then
	rm -rf tmp
fi

mkdir tmp
cp ./src/fuseLib.c tmp/
cp ./src/fuseLib.c mount-point/

./my-fsck disco-virtual
diff ./src/fuseLib.c ./mount-point/fuseLib.c

mv mount-point/fuseLib.c mount-point/fuseLib2.c

./my-fsck disco-virtual
diff ./src/fuseLib.c ./mount-point/fuseLib2.c

chmod ./mount-point/fuseLib2.c 755
./my-fsck disco-virtual
ls -la
