#include "fs.h"
#include "libk.h"
#include "ds/linked.h"

#define TAB_SHIFT 8
#define TAB_SIZE (1 << TAB_SHIFT)

static struct {
	rwlock_t rw;

	struct slist_node buckets[TAB_SIZE];
} inode_cache;

// https://gist.github.com/lh3/974ced188be2f90422cc
static inline uint64_t hash_inode(ino_t ino, dev_t dev)
{
	uint32_t key = (ino + dev), mask = TAB_SIZE - 1;
	key = (~key + (key << 21)) & mask;
	key = key ^ key >> 24;
	key = ((key + (key << 3)) + (key << 8)) & mask;
	key = key ^ key >> 14;
	key = ((key + (key << 2)) + (key << 4)) & mask;
	key = key ^ key >> 28;
	key = (key + (key << 31)) & mask;
	return key;
}

struct inode *inode_get(struct super_block *sb, ino_t ino)
{
	uint64_t hash = hash_inode(ino, sb->dev);

	write_lock(&inode_cache.rw);
	
	// If in list
	// 	Remove from list
	struct inode *cur = slist_entry(inode_cache.buckets[hash].next, struct inode, node);
	for (struct inode *prev = NULL; cur != NULL; cur = slist_get_next(cur, node), prev = cur) {
		if (cur->ino == ino && cur->sb->dev == sb->dev) {
			kref_inc(&cur->rc);

			// Delete from list
			if (prev == NULL)
				inode_cache.buckets[hash].next = NULL;
			else
				slist_set_next(prev, node, slist_get_next(cur, node));
		}
	}

	// Else alloc and read
	if (!cur) {
		kassert(HASOP(sb, alloc_inode));
		cur = sb->ops->alloc_inode(sb);
		kassert(cur != NULL); // TODO: Fix
		cur->ino = ino;

		kassert(HASOP(sb, read_inode));
		err_t rv = sb->ops->read_inode(sb, cur);
		if (rv) {
			klog_warn("vfs", "read_inode fail: %ld\n", rv);

			kassert(HASOP(sb, dealloc_inode));
			sb->ops->dealloc_inode(cur);

			cur = NULL;		
		} else {
			// Add to list
			__slist_append(&inode_cache.buckets[hash], &cur->node);
		}
	}

	write_unlock(&inode_cache.rw);

	return cur;
}
