#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
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
 * file: pointer to the FILE descriptor 
 * buf: parameter to return the read string. Buf is a
 * string passed by reference. 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly "built" in memory, return the starting 
 * address of the string (pointer returned by malloc()) in the buf parameter
 * (*buf)=<address>;
 * 
 * Returns: 0 if success, -1 if error
 */
int
loadstr(FILE * file, char **buf)
{
	char tmp;
	int sz = 0;
	int ok;

	tmp = getc(file);
	while (tmp != '\0') {
		sz++;
		tmp = getc(file);
	}

	ok = fseek(file, -(sz + 1), SEEK_CUR); // -(sz +1) retrocedemos al punto inicial del archivo tras averiguar el tamaño a.txt -> sz = 5. +1 por '\0'
	if (ok != 0) {
		return -1;
	}

	*buf = malloc(sizeof(char) * sz); //Reservamos memoria para almacenar en buf la cadena que vamos a leer
	if (*buf == NULL) {
		return -1;
	}

	ok = fread(*buf, sizeof(char), sz, file);
	if (ok != sz) {
		free(*buf);
		return -1;
	}

	ok = fseek(file, 1, SEEK_CUR); //SUMAMOS UNO A LA DIRECCION ACTUAL DEL PUNTERO DEL ARCHIVO PARA LEER EL SIGUIENTE TITULO DE ARCHIVO
								   //YA QUE EL FREAD DEJA EL PUNTERO EN EL \0
	if (ok != 0) {
		free(*buf);
		return -1;
	}

	return 0;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * header: output parameter. It is used to return the starting memory address
 * of an array that contains the (name,size) pairs read from the tar file
 * nFiles: output parameter. Used to return the number of files stored in
 * the tarball archive (first 4 bytes of the header)
 *
 * On success it returns EXIT_SUCCESS. Upon failure, EXIT_FAILURE is returned.
 * (both macros are defined in stdlib.h).
 */
int
readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles)
{
	int i;
	int ok;
	stHeaderEntry *p;

	ok = fread(nFiles, sizeof(int), 1, tarFile);
	if (ok != 1) {
		return EXIT_FAILURE;
	}

	p = malloc(sizeof(stHeaderEntry) * (*nFiles)); //Reservamos memoria para un array de stHeaderEntry
	if (p == NULL) {
		return EXIT_FAILURE;
	}

	for (i = 0; i < *nFiles; i++) {
		ok = loadstr(tarFile, &((p + i)->name)); //Cargamos p[i].name->a.txt
		if (ok == -1) {
			free(p);
			return EXIT_FAILURE;
		}

		ok = fread(&(p + i)->size, sizeof(int), 1, tarFile); //Cargamos p[i].size
		if (ok != 1) {
			free(p);
			return EXIT_FAILURE;
		}
	}

	*header = p;

	return EXIT_SUCCESS;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
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
	if (tar == NULL) { //
		return EXIT_FAILURE;
	}

	p = malloc(sizeof(stHeaderEntry) * nFiles);
	if (p == NULL) {
		return EXIT_FAILURE;
	}

	ok = fwrite(&nFiles, sizeof(int), 1, tar);
	if (ok != 1) {
		free(p);
		return EXIT_FAILURE;
	}

	for (i = 0; i < nFiles; ++i) {
		///////////////
		//Escribimos el nombre del fichero terminado en '\0'
		//////////////
		(p + i)->name = fileNames[i];
		ok = fwrite((p + i)->name, 1, strlen((p + i)->name), tar);
		if (ok != strlen((p + i)->name)) {
			free(p);
			return EXIT_FAILURE;
		}

		putc('\0', tar);
		tmp = fopen((p + i)->name, "r");
		if (tmp == NULL) {
			free(p);
			return EXIT_FAILURE;
		}
		///////////////////////////////////////////////

		////////////
		//Calculamos el tamaño del fichero y lo escribimos
		////////////
		ok = fseek(tmp, 0L, SEEK_END);
		if (ok != 0) {
			free(p);
			return EXIT_FAILURE;
		}

		sz = ftell(tmp);
		(p + i)->size = sz;
		ok = fclose(tmp);
		if (ok == EOF) {
			free(p);
			return EXIT_FAILURE;
		}

		ok = fwrite(&sz, sizeof(int), 1, tar);
		if (ok != 1) {
			free(p);
			return EXIT_FAILURE;
		}
		//////////////////////////////////////////////
	}

	//////////////////////////////////////////
	//Copiamos la informacion de cada fichero al tar
	/////////////////////////////////////////
	for (i = 0; i < nFiles; ++i) {
		tmp = fopen((p + i)->name, "r");
		if (tmp == NULL) {
			free(p);
			return EXIT_FAILURE;
		}

		ok = copynFile(tmp, tar, (p + i)->size);
		if (ok == -1) {
			free(p);
			return EXIT_FAILURE;
		}

		ok = fclose(tmp);
		if (ok == EOF) {
			free(p);
			return EXIT_FAILURE;
		}
	}

	ok = fclose(tar);
	if (ok == EOF) {
		free(p);
		return EXIT_FAILURE;
	}

	free(p);

	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
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

/** Extract info files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
infoTar(char tarName[])
{
	FILE *tar;
	stHeaderEntry *cabecera;
	int i = 0;
	int *nFiles;
	int ok;

	tar = fopen(tarName, "r");
	if(tar == NULL){
		return EXIT_FAILURE;
	}

	nFiles = malloc(sizeof(int));
	if(nFiles == NULL){
		return EXIT_FAILURE;
	}

	ok = readHeader(tar, &cabecera, nFiles);
	if(ok == EXIT_FAILURE){
		return EXIT_FAILURE;
	}

	for (i = 0; i < *nFiles; ++i) {
		printf("[%d]: ",i);
		printf("file %s, ", (cabecera +i)->name);
		printf("size %d Bytes \n", (cabecera +i)->size);
	}

	ok = fclose(tar);

	if (ok == EOF) {
		free(cabecera);
		return EXIT_FAILURE;
	}

	free(cabecera);
	return EXIT_SUCCESS;
}

/** Merge a tarball archive 
 *
 * nfiles: number of mtar files
 * filenames: array with the path names of the mtar files
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 */
int
mergeTar(int nFiles, char *fileNames[], char tarName[])
{
	int i = 0;
	int j = 0;
	int ok;
	int sum;
	int *nFilesArray;
	FILE *mergeTar;
	FILE *tmp;
	stHeaderEntry *arrayCabeceras;
	stHeaderEntry *aux;
	char *p;

	mergeTar = fopen(tarName,"w+");
	if(mergeTar == NULL){
		return EXIT_FAILURE;
	}

	arrayCabeceras = malloc(sizeof(stHeaderEntry) * nFiles);
	if(arrayCabeceras == NULL){
		return EXIT_FAILURE;
	}
	nFilesArray = malloc(sizeof(int) * nFiles);
	if(nFilesArray == NULL){
		free(arrayCabeceras);
		return EXIT_FAILURE;
	}

	for (i = 0; i < nFiles; ++i) {
		tmp = fopen(fileNames[i],"r");
		if(tmp == NULL){
			free(arrayCabeceras);
			free(nFilesArray);
			return EXIT_FAILURE;
		}
		ok = readHeader(tmp, &arrayCabeceras[i],&nFilesArray[i]);
		sum = sum + nFilesArray[i];
		if(ok == EXIT_FAILURE){
			free(arrayCabeceras);
			free(nFilesArray);
			return EXIT_FAILURE;
		}

		ok = fclose(tmp);
		if (ok == EOF) {
			free(arrayCabeceras);
			free(nFilesArray);
			return EXIT_FAILURE;
		}
	}

	ok = fwrite(&sum, sizeof(int), 1, mergeTar);
	if (ok != 1) {
		free(arrayCabeceras);
		free(nFilesArray);
		return EXIT_FAILURE;
	}

	for(i = 0; i < nFiles;++i){
		for (j = 0; j < nFilesArray[i]; ++j) {
			aux = &arrayCabeceras[i];
			ok = fwrite((aux +j)->name,sizeof(char),strlen((aux +j)->name),mergeTar);
			if (ok != strlen((aux +j)->name)) {
				free(arrayCabeceras);
				free(nFilesArray);
				return EXIT_FAILURE;
			}
			putc('\0',mergeTar);

			ok = fwrite(&(aux +j)->size,sizeof(int),1,mergeTar);
			if(ok != 1){
				free(arrayCabeceras);
				free(nFilesArray);
				return EXIT_FAILURE;
			}
		}
	}
}
