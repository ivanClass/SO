#include "fuseLib.h"
#include "myFS.h"

#include <stdlib.h>
#include <string.h>


MyFileSystem myFileSystem;

#define USAGE			"Usage: %s -t diskSize -a backupFileName -f 'fuse options'\n"
#define EXAMPLE		"Example:\n%s -t 2097152 -a virtual-disk -f '-d -s mount-point'\n"


int main(int argc, char **argv) {
	myFileSystem.numFreeNodes = MAX_NODES;

	int ret; // Resulting code of the functions call

	int opt, diskSize = -1;
	char *backupFileName = NULL;
	char *argsFUSE = NULL;

	char *argvNew[MAX_FUSE_NARGS];
	char *pTmp;

	while((opt = getopt(argc, argv, "t:a:f:")) != -1) {
		switch(opt) {
			case 't':
				diskSize = atoi(optarg);
				break;
			case 'a':
				backupFileName = optarg;
				break;
			case 'f':
				argsFUSE = optarg;
				break;
			default: /* '?' */
				fprintf(stderr, USAGE, argv[0]);
				fprintf(stderr, EXAMPLE, argv[0]);
				exit(-1);
		}
	}


	// Any parameter missing?
	if(diskSize == -1 || backupFileName == NULL || argsFUSE == NULL) {
		fprintf(stderr, USAGE, argv[0]);
		fprintf(stderr, EXAMPLE, argv[0]);
		exit(-1);
	}


	// Format file with out format?
	ret = myMkfs(&myFileSystem, diskSize, backupFileName);
	if(ret) {
		fprintf(stderr, "Unable to format, error code: %d\n", ret);
		exit(-1);
	}
	fprintf(stderr, "File system available\n");

	// Parse the arguments for FUSE
	argc = 1;
	argvNew[0] = argv[0];

	pTmp = strtok(argsFUSE, " ");
	while(pTmp && argc < MAX_FUSE_NARGS) {
		argvNew[argc++] = pTmp;
		pTmp = strtok(0, " ");
	}

	// We mount the FS, exit with Control-C
	if((ret = fuse_main(argc, argvNew, &myFS_operations, NULL))) {
		fprintf(stderr, "Error when mounting the FUSE file system\n");
		return(ret);
	}

	myFree(&myFileSystem);

	return(0);
}
