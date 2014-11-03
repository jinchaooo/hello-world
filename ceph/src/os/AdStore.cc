/*
 * AdStore.cc
 * Author: Jin Chao
 * Email: Jin_Chao@dsi.a-star.edu.sg
 */

#include "include/int_types.h"

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>

#if defined(__linux__)
#include <linux/fs.h>
#endif

#include <iostream>
#include <map>

#include "include/compat.h"
#include "include/linux_fiemap.h"

#include "common/xattr.h"
#include "chain_xattr.h"

#if defined(DARWIN) || defined(__FreeBSD__)
#include <sys/param.h>
#include <sys/mount.h>
#endif // DARWIN


#include <fstream>
#include <sstream>

#include "FileStore.h"
#include "GenericFileStoreBackend.h"
#include "BtrfsFileStoreBackend.h"
#include "XfsFileStoreBackend.h"
#include "ZFSFileStoreBackend.h"
#include "common/BackTrace.h"
#include "include/types.h"
#include "FileJournal.h"

#include "osd/osd_types.h"
#include "include/color.h"
#include "include/buffer.h"

#include "common/Timer.h"
#include "common/debug.h"
#include "common/errno.h"
#include "common/run_cmd.h"
#include "common/safe_io.h"
#include "common/perf_counters.h"
#include "common/sync_filesystem.h"
#include "common/fd.h"
#include "HashIndex.h"
#include "DBObjectMap.h"
#include "KeyValueDB.h"

#include "common/ceph_crypto.h"
using ceph::crypto::SHA1;

#include "include/assert.h"

#include "common/config.h"

#ifdef WITH_LTTNG
#include "tracing/objectstore.h"
#endif

#include "AdStore.h"

AdStore::AdStore(const std::string &cdev,
    const std::string &base, 
    const std::string &jdev,
    osflagbits_t flags,
    const char *internal_name, 
    bool update_to) :
  FileStore(base, jdev, flags, internal_name, update_to),
  cachepath(cdev), 
  cache_lock("AdStore::cache_lock"),
  force_destage(false),
  cache_stop(false),
  cache_lru(0),
  cache_thread(this) 
{
}

AdStore::~AdStore() 
{
}

int AdStore::mkfs()
{
  int r;
  std::string strcache(basedir);
  strcache.append("/current/ad_cache");
  r = FileStore::mkfs();
  ::symlink(cachepath.c_str(), strcache.c_str());
  init_index(coll_t("ad_cache"));
  return r;
}

int AdStore::_write(coll_t cid, const ghobject_t& oid, 
    uint64_t offset, size_t len, 
    const bufferlist& bl, bool replica)
{
  int r;
  coll_t ccid("ad_cache");
  if (exists(ccid, oid)) {
    // already in cache 
    r = FileStore::_write(ccid, oid, 
        offset, len, bl, replica);
    // cache hit, touch the oid in lru cache
    cache_lru.lru_touch(new CItem(cid, oid));
  } else if (len <= CACHE_MAX_LEN) {
    // small write, write to cache
    r = FileStore::_write(ccid, oid, 
        offset, len, bl, replica);
    // add oid in lru cache
    cache_lru.lru_insert_top(new CItem(cid, oid));
  } else {
    // large write, write directly to os
    r = FileStore::_write(cid, oid, 
        offset, len, bl, replica);
  }
  return r;
}

int AdStore::read(coll_t cid, const ghobject_t& oid, 
    uint64_t offset, size_t len, 
    bufferlist& bl, bool allow_eio)
{
  int r;
  coll_t ccid("ad_cache");
  // try to read from cache first
  r = FileStore::read(ccid, oid, 
      offset, len, bl, allow_eio);
  if (r < 0) {
    // cache miss, read from os
    r = FileStore::read(cid, oid, 
        offset, len, bl, allow_eio);
  } else {
    // cache hit, touch the oid in lru cache
    cache_lru.lru_touch(new CItem(cid, oid));
  }
  return r;
}

int AdStore::cache_destage(coll_t cid, 
    const ghobject_t& oid)
{
  int r;
  // FIXME copy the object into os
  FDRef o, n;
  coll_t ccid("ad_cache");
  {
    Index index;
    r = lfn_open(ccid, oid, false, &o, &index);
    if (r < 0) {
      goto out2;
    }
    assert(NULL != (index.index));
    RWLock::WLocker l((index.index)->access_lock);

    r = lfn_open(cid, oid, true, &n, &index);
    if (r < 0) {
      goto out;
    }
    r = ::ftruncate(**n, 0);
    if (r < 0) {
      goto out3;
    }
    struct stat st;
    ::fstat(**o, &st);
    r = _do_clone_range(**o, **n, 0, st.st_size, 0);
    if (r < 0) {
      r = -errno;
      goto out3;
    }
    r = ::ftruncate(**n, st.st_size);
    if (r < 0) {
      goto out3;
    }
  }

#if 0
  dout(20) << "objectmap clone" << dendl;
  r = object_map->clone(oldoid, newoid, &spos);
  if (r < 0 && r != -ENOENT)
    goto out3;
#endif

#define XATTR_SPILL_OUT_NAME "user.cephos.spill_out"
#define XATTR_NO_SPILL_OUT "0"
#define XATTR_SPILL_OUT "1"
  {
    char buf[2];
    map<string, bufferptr> aset;
    r = _fgetattrs(**o, aset);
    if (r < 0)
      goto out3;

    r = chain_fgetxattr(**o, XATTR_SPILL_OUT_NAME, 
        buf, sizeof(buf));
    if (r >= 0 && !strncmp(buf, XATTR_NO_SPILL_OUT, 
          sizeof(XATTR_NO_SPILL_OUT))) {
      r = chain_fsetxattr(**n, XATTR_SPILL_OUT_NAME, 
          XATTR_NO_SPILL_OUT, sizeof(XATTR_NO_SPILL_OUT));
    } else {
      r = chain_fsetxattr(**n, XATTR_SPILL_OUT_NAME, 
          XATTR_SPILL_OUT, sizeof(XATTR_SPILL_OUT));
    }
    if (r < 0)
      goto out3;

    r = _fsetattrs(**n, aset);
    if (r < 0)
      goto out3;
  }

  // FIXME remove the object from cache
  {
    Index index2;
    r = get_index(ccid, &index2);
    if (r < 0)
      return r;
    r = index2->unlink(oid);
  }

out3:
  lfn_close(n);
out:
  lfn_close(o);
out2:
  assert(!m_filestore_fail_eio || r != -EIO);

  return r;
}

int AdStore::cache_load(coll_t cid, 
    const ghobject_t& oid)
{
  return 0;
}

void AdStore::cache_entry()
{
  cache_lock.Lock();
  while (!cache_stop) {
    utime_t max_t;
    max_t.set_from_double(MAX_CACHE_T);
    utime_t min_t;
    min_t.set_from_double(MIN_CACHE_T);

    utime_t start_t = ceph_clock_now(g_ceph_context);

    // wait for at most the max interval
    if (!force_destage) 
      cache_cond.WaitInterval(g_ceph_context, 
          cache_lock, max_t);

    if (force_destage) {
      force_destage = false;
    } else {
      // wait for at least the min interval
      utime_t woke_t = ceph_clock_now(g_ceph_context);
      woke_t -= start_t;
      if (woke_t < min_t) {
        utime_t t = min_t;
        t -= woke_t;
        cache_cond.WaitInterval(g_ceph_context, 
            cache_lock, t);
      }
    }

    // FIXME check if we need to do cache destage

    // FIXME do cache destage, pause thread pool
    op_tp.pause();
    std::cout << "doing cache destage..." << std::endl;
    CItem* p = static_cast<CItem*>
      (cache_lru.lru_get_next_expire());
    if(p) {
      cache_destage(p->cid, p->oid);
      cache_lru.lru_expire();
    }
    op_tp.unpause();
  }
  cache_lock.Unlock();
}

void AdStore::do_force_destage()
{
  Mutex::Locker l(cache_lock);
  force_destage = true;
  cache_cond.Signal();
}

int AdStore::mount() 
{
  int ret;
  ret = FileStore::mount();
  if (ret < 0) 
    return ret;

  // FIXME set lru max 
  cache_lru.lru_set_max(102400);
  cache_thread.create();
  return 0;
}

int AdStore::umount() 
{
  FileStore::umount();
  // wait for the cache thread to finish
  cache_stop = true;
  cache_thread.join();
  return 0;
}
