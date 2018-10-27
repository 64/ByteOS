#include "fs.h"
#include "fs/initrd.h"
#include "mm.h"
#include "libk.h"
#include "multiboot2.h"

static struct inode *initrd_alloc_inode(struct super_block *super)
{
	struct initrd_inode *rd_inode = kmalloc(sizeof *rd_inode, KM_NONE);
	memset(rd_inode, 0, sizeof *rd_inode);
	rd_inode->i.sb = super;
	return &rd_inode->i;
}

static void initrd_dealloc_inode(struct inode *inode)
{
	struct initrd_inode *rd_inode = container_of(inode, struct initrd_inode, i);
	if (rd_inode != NULL)
		kfree(rd_inode);
}

static err_t initrd_read_inode(struct super_block *sb, struct inode *inode)
{
	if (inode->ino < INITRD_ROOT_INO)
		return ENOENT;
	else if (inode->ino == INITRD_ROOT_INO) {
		inode->ops = &initrd_inode_dir_ops;
		inode->mode = I_DIRECTORY;
		inode->sb = sb;
		return 0;
	}


	struct initrd_fs *fs = initrd_sb_container(sb);
	kassert_dbg(inode->sb == &fs->super);

	// This inode is a file/directory/symlink, parse the ustar memory
	struct ustar_header *hdr = ustar_nth_from_entry(fs->root_hdr, inode->ino - INITRD_ROOT_INO);

	if (hdr == NULL)
		return ENOENT;

	const mode_t mode_table[] = { I_REGULAR, 0, I_SYMLINK, 0, 0, I_DIRECTORY, 0 };
	kassert_dbg(hdr->type >= '0' && hdr->type <= '6');
	mode_t mode = mode_table[hdr->type - '0'];
	kassert(mode != 0);

	// Write to the inode
	inode->mode = mode;
	inode->ops = (mode == I_DIRECTORY) ? &initrd_inode_dir_ops : &initrd_inode_file_ops;

	return 0;
}

static struct super_operations initrd_super_ops = {
	.alloc_inode = initrd_alloc_inode,
	.read_inode = initrd_read_inode,
	.write_inode = NULL,
	.dealloc_inode = initrd_dealloc_inode
};

struct super_block *initrd_get_super(void)
{
	kassert_dbg(sizeof(struct ustar_header) == 512);

	struct initrd_fs *fs = kmalloc(sizeof *fs, KM_NONE);
	memset(fs, 0, sizeof *fs);

	struct inode *inode = kmalloc(sizeof *inode, KM_NONE);
	memset(inode, 0, sizeof *inode);

	fs->super.dev = 1;
	fs->super.root = inode;
	fs->super.ops = &initrd_super_ops;
	
	// Create the root directory
	fs->root = inode_get(&fs->super, INITRD_ROOT_INO);

	return &fs->super;
}
