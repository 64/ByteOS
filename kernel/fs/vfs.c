#include "err.h"
#include "fs.h"
#include "fs/initrd.h"
#include "libk.h"

struct inode vfs_root;

void vfs_init(void)
{
	memset(&vfs_root, 0, sizeof vfs_root);
	err_t e = vfs_mount(&vfs_root, 1);
	if (e)
		panic("Failed to mount root fs");
}

// Overwrites the inode to mount the filesystem at this point
err_t vfs_mount(struct inode *mount_point, dev_t dev)
{
	if (I_ISMNT(mount_point))
		return EBUSY;

	// Get superblock from device number
	struct super_block *sb;
	if (dev == 1) { // TODO: Don't hardcode
		// Use initrd
		sb = initrd_get_super();
	} else
		panic("Unknown device %d", dev);

	mount_point->sb = sb;
	mount_point->mounted = sb->root;
	mount_point->mode = I_MOUNT;

	return 0;
}

// Must call inode_put after the result is used
err_t vfs_lookup(struct inode *dir, const char *path, size_t len, struct inode **result)
{
	*result = NULL;

	if (!I_ISDIR(dir))
		return ENOTDIR;
	else if (!HASOP(dir, lookup))
		return ENOTSUP;
	return dir->ops->lookup(dir, path, len, result);
}
