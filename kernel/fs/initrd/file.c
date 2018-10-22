#include "fs.h"
#include "fs/initrd.h"

struct inode_operations initrd_inode_file_ops = {
	.lookup = NULL // This is only for directories (see initrd/dir.c)
};

