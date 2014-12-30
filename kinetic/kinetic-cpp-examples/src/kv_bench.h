#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <vector>
#include <boost/scoped_ptr.hpp>
//#include "common/ceph_context.h"
//#include "os/ObjectStore.h"

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>
#include "kinetic/kinetic.h"
#include "glog/logging.h"
#include <sys/time.h>

#define DEFAULT_BENCH_NAME "KVBench"
#define NBC std::shared_ptr<kinetic::NonblockingKineticConnection>
class KVBencher {
  public:
    KVBencher(NBC conn, std::string name):bench_name(name) {
      this->conn = conn;
      if (bench_name.empty())
        bench_name = std::string(DEFAULT_BENCH_NAME);
      std::stringstream ss;
      ss<<bench_name<<"_Object";
      bench_pref = ss.str();
    }
    int init_write(uint32_t count,
        uint32_t min_order, uint32_t max_order, bool do_cleanup=false);
    int sequential_bench(uint32_t write_percentage=0);
    int random_bench(uint32_t count, 
        const std::string& rule, uint32_t write_percentage=0);
    void cleanup(bool fetch=true);
  private:
    int fetch_bench_metadata();
    int save_bench_metadata();
    NBC conn;
    std::string bench_name;    // for saving or fetching benchmark metadata
    std::string bench_pref;    // prefix for object name
    uint32_t object_count;     // total object to write, read
    uint32_t min_size_order, max_size_order; 
    std::vector<uint64_t> object_sizes;  
};
