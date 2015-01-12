#include <stdio.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"

#include <sys/time.h>
#include "kv_bench.h"
#include <vector>
#include <thread>

// global kinetic connection options
kinetic::ConnectionOptions options;

void bench_thread(uint32_t idx, uint32_t count, 
    uint32_t minorder, uint32_t maxorder, bool cleanup,
    uint64_t* total_bytes, double* ti_sec)
{
  // create the connection with kv server
  kinetic::KineticConnectionFactory kcf = 
    kinetic::NewKineticConnectionFactory();

  std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;

  if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
    std::cout << "Unable to connect to kv server\n";
    return;
  }

  std::stringstream ss;
  ss << DEFAULT_BENCH_NAME;
  ss << idx;
  std::string bench_name(ss.str());
  KVBencher bencher(nbc, bench_name);    
  bencher.init_write_noprint(count, minorder, maxorder, cleanup, 
      total_bytes, ti_sec);
}

int main(int argc, char* argv[])
{
  if (argc < 4) {
    std::cout << "usage: "
      << argv[0]
      << " <server_ip>"
      << " <server_port>"
      << " init/sequential/random/cleanup"
      << " [...]"
      << std::endl;
    return -1;
  }

  std::string prefix(argv[3]);

  if (prefix == "init") {
    if (argc < 8) {
      std::cout << "usage: "
        << argv[0]
        << " <server_ip>"
        << " <server_port>"
        << " init"
        << " <thread_num>"
        << " <count>"
        << " <minorder>"
        << " <maxorder>"
        << " [cleanup]"
        << std::endl;
      return -1;
    }
    const char* host = argv[1];
    int port = atoi(argv[2]);
    uint32_t thread_num = (uint32_t)atoi(argv[4]);
    uint32_t count = (uint32_t)atoi(argv[5]);
    uint32_t minorder = (uint32_t)atoi(argv[6]);
    uint32_t maxorder = (uint32_t)atoi(argv[7]);
    bool do_cleanup = false;
    if (argc > 8) {
      std::string cleanup(argv[8]);
      if (cleanup.compare("cleanup") == 0)
        do_cleanup = true;
    }

    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    // each thread creates a connection with kv server
    std::vector<std::unique_ptr<std::thread>> threads;
    uint64_t total_bytes_thread[thread_num];
    double ti_sec_thread[thread_num];

    for (uint32_t i = 0; i < thread_num; i++) {
      std::unique_ptr<std::thread> t(new std::thread(bench_thread,
            i, count, minorder, maxorder, do_cleanup,
            &total_bytes_thread[i], &ti_sec_thread[i]));
      threads.push_back(std::move(t));
    }

    // wait for all threads to complete
    for (auto c = threads.begin(); c != threads.end(); ++c) 
      (*c)->join();

    std::cout << "All bench threads finished\n";
    double oprate_all_threads = 0;
    double datarate_all_threads = 0;
    
    for (uint32_t i = 0; i < thread_num; i++) {
      std::cout << "\nThread " << i << ":\n";
      std::cout << "Execution time is: " << ti_sec_thread[i] << " secs\n";
      double oprate = count / ti_sec_thread[i];
      std::cout << "Operations per sec is: "<< oprate << "\n";
      std::cout << "Write bytes: "<< total_bytes_thread[i] << "\n";
      double datarate = total_bytes_thread[i] / ti_sec_thread[i];
      datarate = datarate / 1024;
      std::cout << "Throughput is: "<< datarate << " KB per sec\n";
      oprate_all_threads += oprate;
      datarate_all_threads += datarate;
    }
    std::cout << "\nAggragate Performance:\n";
    std::cout << "Operations per sec is: "<< oprate_all_threads << "\n";
    std::cout << "Throughput is: "<< datarate_all_threads << " KB per sec\n";

  } else if (prefix == "sequential") {
    std::cout << "Not supported yet\n";
  } else if (prefix == "random") {
    std::cout << "Not supported yet\n";
  } else if (prefix == "cleanup") {
    std::cout << "Not supported yet\n";
  } else {
    std::cout << "Invalid prefix. "
      << "Should be init/sequential/random/cleanup.\n";
    return -1;
  }

  return 0;
}
