#!/bin/bash

if test -e mytar && test -x mytar; then
	if test -e tmp && test -d tmp; then #####COMPROBAMOS SI EXISTE TMP Y ES UN DIRECTORIO
		rm -rf tmp
	fi
	
	mkdir tmp
	cd tmp

	##CREAMOS EL PRIMER ARCHIVO
	touch file1.txt
	echo "Hello world!" > file1.txt

	##CREAMOS EL SEGUNDO ARCHIVO
	touch file2.txt
	head -n 10 /etc/passwd > file2.txt
	
	##CREAMOS EL TERCER ARCHIVO
	touch file3.dat
	head -c 1024 /dev/urandom > file3.dat

	##INVOCAMOS NUESTRO PROGRAMA mytar
	../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat

	##CREAMOS DIRECTORIO OUT
	mkdir out
	cp filetar.mtar out
	
	cd out
	../../mytar -x -f filetar.mtar
	
	let NUM0=0
	let NUM1=0

	if !(diff file1.txt ../file1.txt); then
		echo "El archivo file1.txt no es igual"
		NUM0=1
	fi

	if !(diff file2.txt ../file2.txt); then
		echo "El archivo file2.txt no es igual"
		NUM0=1
	fi

	if !(diff file3.dat ../file3.dat); then
		echo "El archivo file3.dat no es igual"
		NUM0=1
	fi
	
	cd ../..
	if [ $NUM0 -eq $NUM1 ]; then
		echo "Correct"
	fi
	
	exit $NUM0

else
	echo "mytar no existe o no es ejecutable."
fi
	
