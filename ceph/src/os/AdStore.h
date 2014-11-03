/*
 * AdStore.h
 * Author: Jin Chao
 * Email: Jin_Chao@dsi.a-star.edu.sg
 */

#ifndef ADSTORE_H
#define ADSTORE_H

#include "include/types.h"

#include <map>
#include <deque>
#include <boost/scoped_ptr.hpp>
#include <fstream>
using namespace std;

#include "include/unordered_map.h"

#include "include/assert.h"

#include "ObjectStore.h"
#include "JournalingObjectStore.h"

#include "common/Timer.h"
#include "common/WorkQueue.h"

#include "common/Mutex.h"
#include "HashIndex.h"
#include "IndexManager.h"
#include "ObjectMap.h"
#include "SequencerPosition.h"
#include "FDCache.h"
#include "WBThrottle.h"

#include "include/uuid.h"

#include "FileStore.h"
#include "include/lru.h"

#define CACHE_MAX_LEN (1*1024*1024)
#define MAX_CACHE_T 30.0 
#define MIN_CACHE_T 0.0

class CItem : public LRUObject
{
  public:
    coll_t cid;
    ghobject_t oid;
    CItem(coll_t& c_id, const ghobject_t& o_id) 
      : cid(c_id), oid(o_id) {}
};

class AdStore : public FileStore
{
  std::string cachepath;
  public:
  AdStore(const std::string &cdev,
      const std::string &base, 
      const std::string &jdev,
      osflagbits_t flags = 0,
      const char *internal_name = "filestore", 
      bool update_to = false);
  ~AdStore();

  virtual int mkfs();
  // make it virtual in filestore
  virtual int _write(coll_t cid, const ghobject_t& oid, 
      uint64_t offset, size_t len, 
      const bufferlist& bl, bool replica = false);
  virtual int read(coll_t cid, const ghobject_t& oid, 
      uint64_t offset, size_t len, 
      bufferlist& bl, bool allow_eio = false);

  Mutex cache_lock;
  Cond cache_cond;
  bool force_destage;
  bool cache_stop;
  LRU cache_lru;

  // move object from cache to os
  int cache_destage(coll_t cid, const ghobject_t& oid);
  // load object from os to cache
  int cache_load(coll_t cid, const ghobject_t& oid);
  void cache_entry();
  void do_force_destage();
  struct CacheThread : public Thread {
    AdStore* ads;
    CacheThread(AdStore* ad) : ads(ad) {
    }
    void* entry() {
      ads->cache_entry();
      return 0;
    }
  } cache_thread;

  virtual int mount();
  virtual int umount();
  private:

  friend class FileStoreBackend;
  friend class TestFileStore;
};

#endif
