#include "fs.h"
#include "fs/initrd.h"
#include "libk.h"

static err_t initrd_dir_lookup_inode(struct inode *dir_inode, const char *name, size_t len, struct inode **out)
{
	kassert_dbg(dir_inode && I_ISDIR(dir_inode) && dir_inode->sb != NULL);
	struct initrd_fs *fs = initrd_sb_container(dir_inode->sb);

	const char *dir_name;
	if (dir_inode == fs->root)
		dir_name = "/";
	else
		dir_name = initrd_inode_container(dir_inode)->hdr->filename;

	size_t dir_len = strlen(dir_name);

	// Scan through the headers, comparing file names against dir->hdr->filename
	for (struct ustar_header *cur = fs->root_hdr; cur != NULL; cur = ustar_nth_from_entry(cur, 1)) {
		size_t target_len = strlen(cur->filename);

		if (target_len < dir_len || target_len > dir_len + len + 1)
			continue;

		// If both path components match, we're done
		if (memcmp(dir_name, cur->filename, dir_len) == 0
			&& memcmp(name, cur->filename + dir_len, len) == 0) {
			return 0;		
		}
	}

	// Lookup failure is not an error (or, as I like to call it, LFINAE)
	*out = NULL;
	return 0; 
}

struct inode_operations initrd_inode_dir_ops = {
	.lookup = initrd_dir_lookup_inode
};
