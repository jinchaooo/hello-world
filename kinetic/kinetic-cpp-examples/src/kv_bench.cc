#include "kv_bench.h"
//#include "osd_types.h"

#include <stdio.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

#include <sys/time.h>

//static coll_t KV_BENCH_COLL("kv_bench");

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::PutCallbackInterface;
using kinetic::KineticStatus;

using std::make_shared;
using std::unique_ptr;

// callback for put ops
class PutCallback : public PutCallbackInterface {
  public:
    PutCallback(int* remaining) : remaining_(remaining) {};
    void Success() {
      printf(".");
      fflush(stdout);
      (*remaining_)--;
    }
    void Failure(KineticStatus error) {
      printf("Error: %d %s\n", 
          static_cast<int>(error.statusCode()), 
          error.message().c_str());
      exit(1);
    }
  private:
    int* remaining_;
};

int KVBencher::init_write(uint32_t count, 
    uint32_t min_order, uint32_t max_order, bool do_cleanup)
{
  if (min_order == 0 || max_order == 0)
    return 1;
  if (max_order < min_order) {
    uint32_t t = max_order;
    max_order = min_order;
    min_order = t;
  }
  object_count = count;
  min_size_order = min_order;
  max_size_order = max_order;
  object_sizes.clear();

  int remaining = 0;
  fd_set read_fds, write_fds;
  int num_fds = 0;
  auto callback = make_shared<PutCallback>(&remaining);

  uint32_t small = 0, big = 0, mod = max_order - min_order + 1;
  uint64_t total_bytes = 0, total_write_bytes, total_read_bytes;

  // record the time
  timeval begint, endt;
  gettimeofday(&begint, NULL);

  for (uint32_t i = 0; i < object_count; ++i) {
    uint32_t order = min_order + (rand() % mod);
    if (order < 20)
      ++small;
    else
      ++big;
    uint64_t bsize = (uint64_t) 1 << order;
    object_sizes.push_back(bsize);

    std::stringstream tss;
    tss<<bench_pref<<"_"<<i;
    std::string key(tss.str());

    // value of the put ops
    std::string value(bsize, 'a');
    auto record = make_shared<KineticRecord>(value, "", "", 
        Message_Algorithm_SHA1);
    
    remaining++;
    conn->Put(key, "", kinetic::WriteMode::IGNORE_VERSION, record,
        callback, kinetic::PersistMode::WRITE_BACK);
    conn->Run(&read_fds, &write_fds, &num_fds);

    total_bytes += bsize;
  }

  // wait for the ops to finish
  while (remaining > 0) {
    while (select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
    conn->Run(&read_fds, &write_fds, &num_fds);
  }
  
  gettimeofday(&endt, NULL);

  if (do_cleanup) {
    cleanup(false);
  } else {
    save_bench_metadata();
  }

  total_write_bytes = total_bytes;
  total_read_bytes = 0;
  double ti_sec;
  double oprate;
  double datarate;
  ti_sec = endt.tv_sec - begint.tv_sec;
  ti_sec += (endt.tv_usec - begint.tv_usec) / (1000.0 * 1000.0);
  std::cout << std::endl << "Done!" << std::endl;
  std::cout << "Total time is: " << ti_sec << " secs" << std::endl;
  oprate = count / ti_sec;
  std::cout << "Operations per sec is: "<< oprate << std::endl;
  std::cout << "Total write bytes: " << total_write_bytes << std::endl;
  std::cout << "Total read bytes: " << total_read_bytes << std::endl;
  datarate = total_write_bytes / ti_sec;
  std::cout << "Throughput is: "<< datarate << " bytes per sec" << std::endl;

  return 0;
}

int KVBencher::sequential_bench(uint32_t write_percentage)
{
  return 0;
}

int KVBencher::random_bench(uint32_t count, 
    const std::string& rule, uint32_t write_percentage)
{
  return 0;
}
void KVBencher::cleanup(bool fetch)
{
}

int KVBencher::fetch_bench_metadata()
{
  return 0;
}

int KVBencher::save_bench_metadata()
{
  return 0;
}
