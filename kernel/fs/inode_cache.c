#include "fs.h"
#include "libk.h"
#include "mutex.h"
#include "ds/hash.h"

#define TAB_SHIFT 8
#define TAB_SIZE (1 << TAB_SHIFT)

static struct {
	mutex_t mutex;

	struct hlist_bucket buckets[TAB_SIZE];
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
	struct inode *inode = NULL;
	struct hlist_bucket *bucket = &inode_cache.buckets[hash];

	mutex_lock(&inode_cache.mutex);
	
	// If in list, return pointer
	hlist_foreach(cur, struct inode, node, bucket) {
		if (cur->ino == ino && cur->sb->dev == sb->dev) {
			inode = cur;
			break;
		}
	}

	// Else alloc and read
	if (inode == NULL) {
		kassert_dbg(HASOP(sb, alloc_inode));
		inode = sb->ops->alloc_inode(sb);
		kassert(inode != NULL); // TODO: Add kmalloc can't fail requirement
		inode->ino = ino;

		kassert(HASOP(sb, read_inode));
		err_t rv = sb->ops->read_inode(sb, inode);
		if (rv) {
			klog_warn("vfs", "read_inode fail: %ld\n", rv);

			kassert_dbg(HASOP(sb, dealloc_inode));
			sb->ops->dealloc_inode(inode);

			inode = NULL;		
		} else {
			// Add to list
			kassert_dbg(kref_read(&inode->rc) == 0);
			hlist_add(bucket, &inode->node);
		}
	}

	kref_inc(&inode->rc);

	mutex_unlock(&inode_cache.mutex);

	return inode;
}

void inode_put(struct inode *inode)
{
	kprintf("%p\n", inode);
	uint64_t hash = hash_inode(inode->ino, inode->sb->dev);

	mutex_lock(&inode_cache.mutex);

	if (kref_dec_read(&inode->rc) == 0) {
		// No references left, delete this from the cache safely
		hlist_remove(&inode_cache.buckets[hash], &inode->node);

		kassert_dbg(HASOP(inode->sb, dealloc_inode));
		inode->sb->ops->dealloc_inode(inode);
	}

	mutex_unlock(&inode_cache.mutex);

	// TODO: Delete inode from underlying superblock
}
