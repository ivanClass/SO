#!/bin/bash

##A)COMPROBAMOS SI YA EXISTE TMP Y SI ES UN DIRECTORIO -> BORRAMOS
if test -e tmp && test -d tmp; then
	rm -rf tmp
fi

##A)CREAMOS EL DIRECTORIO Y COPIAMOS A TMP Y A SF
mkdir tmp
cp ./src/fuseLib.c tmp/
cp ./src/myFS.h tmp/
cp ./src/fuseLib.c punto-montaje/
cp ./src/myFS.h punto-montaje/

##B)AUDITAMOS EL DISCO Y TRUNCAMOS fuseLib.c PARA QUE OCUPE UN BLOQUE MENOS
./my-fsck disco-virtual

diff ./src/fuseLib.c ./punto-montaje/fuseLib.c
diff ./src/myFS.h ./punto-montaje/myFS.h

truncate -o -s -1 ./punto-montaje/fuseLib.c ##LE QUITAMOS UN BLOQUE
truncate -o -s -1 ./tmp/fuseLib.c

##C)AUDITAMOS EL DISCO Y HACEMOS DIFF ENTRE EL ORIGINAL Y EL TRUNCADO
./my-fsck disco-virtual
diff ./src/fuseLib.c ./punto-montaje/fuseLib.c

##D,E)COPIAMOS UN TERCER FICHERO,AUDITAMOS DISCO Y HACEMOS DIFF

cp ./src/MyFileSystem.c punto-montaje/
./my-fsck disco-virtual
diff ./src/MyFileSystem.c ./punto-montaje/MyFileSystem.c

##F)TRUNCAMOS myFS.h PARA QUE OCUPE ALGUN BLOQUE MÁS
truncate -o -s +1 ./punto-montaje/myFS.h
truncate -o -s +1 ./tmp/myFS.h

##G)AUDITAMOS DISCO Y HACEMOS DIFF
./my-fsck disco-virtual
diff ./src/myFS.h ./punto-montaje/myFS.h

