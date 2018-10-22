#pragma once

#include <stdint.h>

#include "fs.h"
#include "mm_types.h"

#define INITRD_ROOT_INO 1

struct initrd_inode {
	struct ustar_header *hdr;
	struct inode i;
};

struct initrd_fs {
	struct super_block super;
	struct ustar_header *root_hdr;
	struct inode *root;
};

struct ustar_header {
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char type;
	char __padding[512 - 157]; // Other fields
} __attribute__((packed));

static inline struct initrd_inode *initrd_inode_container(struct inode *inode)
{
	return inode == NULL ? NULL : container_of(inode, struct initrd_inode, i);
}

static inline struct initrd_fs *initrd_sb_container(struct super_block *super)
{
	return super == NULL ? NULL : container_of(super, struct initrd_fs, super);
}

extern struct inode_operations initrd_inode_file_ops;
extern struct inode_operations initrd_inode_dir_ops;

struct super_block *initrd_get_super(void);

uint32_t ustar_oct_to_bin(const char *str, size_t len);
struct ustar_header *ustar_nth_from_entry(struct ustar_header *root_hdr, uint32_t n);
virtaddr_t ustar_file_start(struct ustar_header *hdr);
