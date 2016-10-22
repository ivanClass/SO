#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"
extern char *use;

/** Copy the nBytes origin file to destination file.
 *
 * origin: file descriptor pointer FILE of origin
 * destination:  file descriptor pointer FILE of destination
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes copied or -1 if error.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	char *pm;
	int ok;

	/* En pm almacenamos la cadena de caracteres que vamos a leer del fichero origin */
	pm = malloc(sizeof(char) * nBytes);
	if(pm == NULL)
		return -1;

	ok = fread(pm, sizeof(char), nBytes, origin);
	if(ok != nBytes){
		free(pm); // Si la lectura ha fallado, liberamos memoria de pm
		return -1;
	}


	ok = fwrite(pm, sizeof(char), nBytes, destination);
	if(ok != nBytes){
		free(pm); // Si la escritura ha fallado, liberamos memoria de pm
		return -1;
	}

	free(pm);

	return nBytes;
}

/** Loads a string from a file.
 *
 * file: file descriptor pointer to FILE structure
 * buf: address pointer that is initialized with the address where
 * the string is copied. It will be the heap address obtained with malloc.
 *
 * Returns: 0 if success, -1 if error
 */
int loadstr(FILE *file, char **buf)
{
	char tmp;
	int sz = 0;
	int ok;


	tmp = getc(file);
	while(tmp != '\0'){
		sz++;
		tmp = getc(file);
	}

	ok = fseek(file, -(sz + 1), SEEK_CUR); // -(sz +1) retrocedemos al punto inicial del archivo tras averiguar el tamaño a.txt -> sz = 5. +1 por '\0'
	if(ok != 0){
		return -1;
	}

	*buf = malloc(sizeof(char) * sz); //Reservamos memoria para almacenar en buf la cadena que vamos a leer
	if(*buf == NULL){
		return -1;
	}

	ok = fread(*buf, sizeof(char), sz, file);
	if(ok != sz){
		free(*buf);
		return -1;
	}

	ok = fseek(file, 1, SEEK_CUR); //SUMAMOS UNO A LA DIRECCION ACTUAL DEL PUNTERO DEL ARCHIVO PARA LEER EL SIGUIENTE TITULO DE ARCHIVO
								   //YA QUE EL FREAD DEJA EL PUNTERO EN EL \0
	if(ok != 0){
		free(*buf);
		return -1;
	}

	return 0;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: FILE descriptor pointer of the tarball
 * header: pointer address that is initialized with the buffer address
 * that contains the descriptors of the files loaded. 
 * nFiles: integer address (integer pointer) that is initialized with #files
 * contained in the tarball (first bytes of header)
 *
 * Returs: EXIT_SUCCESS if success or EXIT_FAILURE if error
 * (macros defined in stdlib.h).
 */
int readHeader(FILE *tarFile, stHeaderEntry **header, int *nFiles)
{
	int i;
	int ok;
	stHeaderEntry *p;


	ok = fread(nFiles, sizeof(int), 1, tarFile);
	if(ok != 1){
		return EXIT_FAILURE;
	}

	p = malloc(sizeof(stHeaderEntry) * (*nFiles)); //Reservamos memoria para un array de stHeaderEntry
	if(p == NULL){
		return EXIT_FAILURE;
	}

	for(i = 0; i < *nFiles; i++){
		ok = loadstr(tarFile, &((p+i)->name)); //Cargamos p[i].name->a.txt
		if(ok == -1){
			free(p);
			return EXIT_FAILURE;
		}

		ok = fread(&(p+i)->size, sizeof(int), 1, tarFile); //Cargamos p[i].size
		if(ok != 1){
			free(p);
			return EXIT_FAILURE;
		}
	}

	*header = p;

	return EXIT_SUCCESS;
}

/** build a tarball with crea un tarball a partir de unos ficheros de entrada.
 *
 * nfiles:   #input files
 * filenames: array with input the files' name (paths)
 * tarname: tarball name
 * 
 * Returns EXIT_SUCCESS if success and EXIT_FAILURE if error
 * (macros defined in stdlib.h).
 *
 * HELP: fist reserve room for the tarball header,
 * jump this header size and copy one by one all files,
 * filling at the same time tallbar representation header at memory space.
 * Finally, copy tarball from memory space to header file.
 *
 * Reminder: it is not valid a simple sizeof of stHeaderEntry to
 * calculate the room needed, because sizeof not compute a string size,
 * it's only obtained the pointer size. To compute string size should be used
 * strlen function.
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE *tar; //tar va apuntando al final del archivo
	FILE *tmp;
	stHeaderEntry *p;
	int i;
	int ok;
	int sz;


	tar = fopen(tarName, "a");
	if(tar == NULL){ //
		return EXIT_FAILURE;
	}

	p = malloc(sizeof(stHeaderEntry) * nFiles);
	if(p == NULL){
		return EXIT_FAILURE;
	}

	ok = fwrite(&nFiles, sizeof(int), 1, tar);
	if(ok != 1){
		free(p);
		return EXIT_FAILURE;
	}



	for (i = 0; i < nFiles; ++i) {
		///////////////
		//Escribimos el nombre del fichero terminado en '\0'
		//////////////
		(p + i)->name = fileNames[i];
		ok = fwrite((p+i)->name, 1, strlen((p+i)->name), tar);
		if(ok != strlen((p+i)->name)){
			free(p);
			return EXIT_FAILURE;
		}

		putc('\0',tar);
		tmp = fopen((p+i)->name, "r");
		if(tmp == NULL){
			free(p);
			return EXIT_FAILURE;
		}
		///////////////////////////////////////////////

		////////////
		//Calculamos el tamaño del fichero y lo escribimos
		////////////
		ok = fseek(tmp, 0L, SEEK_END);
		if(ok != 0){
			free(p);
			return EXIT_FAILURE;
		}

		sz = ftell(tmp);
		(p + i)->size = sz;
		ok = fclose(tmp);
		if(ok == EOF){
			free(p);
			return EXIT_FAILURE;
		}

		ok = fwrite(&sz, sizeof(int), 1, tar);
		if(ok != 1){
			free(p);
			return EXIT_FAILURE;
		}
		//////////////////////////////////////////////
	}

	//////////////////////////////////////////
	//Copiamos la informacion de cada fichero al tar
	/////////////////////////////////////////
	for (i = 0; i < nFiles; ++i) {
		tmp = fopen((p+i)->name, "r");
		if(tmp == NULL){
			free(p);
			return EXIT_FAILURE;
		}

		ok = copynFile(tmp, tar, (p+i)->size);
		if(ok == -1){
			free(p);
			return EXIT_FAILURE;
		}

		ok = fclose(tmp);
		if(ok == EOF){
			free(p);
			return EXIT_FAILURE;
		}
	}


	ok = fclose(tar);
	if(ok == EOF){
		free(p);
		return EXIT_FAILURE;
	}

	free(p);

	return EXIT_SUCCESS;
}

/** Extract all file from a tarball
 *
 * tarName: string with tarball's name
 *
 * Returns EXIT_SUCCESS if success and EXIT_FAILURE if error (macros
 * defined in stdlib.h).
 *
 * HELP: first load the header, it also means that we will place at
 * start of the first file stored. Then, extract file[s] info
 * from the header (files name and bytes to be read)
 *
 */
int extractTar(char tarName[])
{
	FILE *tar;
	FILE *tmp;
	stHeaderEntry *cabecera;
	//char *p;
	int *nfiles;
	int i;
	int ok;

	tar = fopen(tarName, "r");
	if(tar == NULL){
		return EXIT_FAILURE;
	}
	nfiles = malloc(sizeof(int));
	if(nfiles == NULL){
		return EXIT_FAILURE;
	}

	////////////////////////////////
	//Leemos la cabecera del tar
	/////////////////////////////////
	ok = readHeader(tar, &cabecera, nfiles);
	if(ok == EXIT_FAILURE){
		free(nfiles);
		return EXIT_FAILURE;
	}
	////////////////////////////////


	////////////////////////////////
	//Por cada archivo del tar:
	// 1. Lo abrimos.
	// 2. Copiamos la información
	////////////////////////////////
	for (i = 0; i < *nfiles; ++i) {
		tmp = fopen((cabecera + i)->name,"w");
		if(tmp == NULL){
			free(nfiles);
			free(cabecera);
			return EXIT_FAILURE;
		}
		ok = copynFile(tar, tmp, (cabecera + i)->size);
		if(ok == -1){
			free(nfiles);
			free(cabecera);
			return EXIT_FAILURE;
		}

		ok = fclose(tmp);
		if(ok == EOF){
			free(nfiles);
			free(cabecera);
			return EXIT_FAILURE;
		}
	}

	free(nfiles);
	free(cabecera);
	ok = fclose(tar);
	if(ok == EOF){
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
