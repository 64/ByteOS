#pragma once

#include "types.h"
#include "atomic.h"
#include "limits.h"
#include "err.h"
#include "sync.h"
#include "ds/linked.h"

#define HASOP(value, op) ((value)->ops && (value)->ops->op)

struct super_block {
	dev_t dev;
	struct inode *root;
	struct super_operations *ops;
};

struct super_operations {
	struct inode *(*alloc_inode)(struct super_block *);
	err_t (*write_inode)(struct super_block *, struct inode *);
	err_t (*read_inode)(struct super_block *, struct inode *);
	void (*dealloc_inode)(struct inode *);
};

#define I_UNKNOWN 0
#define I_REGULAR 1
#define I_DIRECTORY 2
#define I_SYMLINK 3
#define I_MOUNT 4

#define I_ISREG(inode) ((inode)->mode == I_REGULAR)
#define I_ISDIR(inode) ((inode)->mode == I_DIRECTORY)
#define I_ISSYM(inode) ((inode)->mode == I_SYMLINK)
#define I_ISMNT(inode) ((inode)->mode == I_MOUNT)

struct inode {
	ino_t ino;
	mode_t mode;

	kref_t rc;

	struct super_block *sb;
	struct inode_operations *ops;

	struct inode *mounted; // When mode is I_MOUNT, this points to the mounted inode
	struct dlist_node node; // Used by inode_cache
};

struct inode_operations {
	err_t (*lookup)(struct inode *dir, const char *name, size_t len, struct inode **out);
	/*err_t (*follow_symlink)(struct inode *node, const char **out);
	err_t (*link)(struct inode *dir, struct inode *node, const char *name, size_t len);
	err_t (*unlink)(struct inode *dir, struct inode *node, const char *name, size_t len);*/
};

extern struct inode vfs_root;

struct inode *inode_get(struct super_block *, ino_t);
void inode_put(struct inode *);

void vfs_init(void);
RETURNS_ERROR vfs_mount(struct inode *mount_point, dev_t dev);
RETURNS_ERROR vfs_lookup(struct inode *dir, const char *path, size_t len, struct inode **result);
