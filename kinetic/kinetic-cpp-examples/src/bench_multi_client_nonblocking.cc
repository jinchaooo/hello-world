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
    if (argc < 7) {
      std::cout << "usage: "
        << argv[0]
        << " <server_ip>"
        << " <server_port>"
        << " init"
        << " <count>"
        << " <minorder>"
        << " <maxorder>"
        << " [cleanup]"
        << std::endl;
      return -1;
    }
    const char* host = argv[1];
    int port = atoi(argv[2]);
    uint32_t count = (uint32_t)atoi(argv[4]);
    uint32_t minorder = (uint32_t)atoi(argv[5]);
    uint32_t maxorder = (uint32_t)atoi(argv[6]);
    bool do_cleanup = false;
    if (argc > 7) {
      std::string cleanup(argv[7]);
      if (cleanup.compare("cleanup") == 0)
        do_cleanup = true;
    }

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    // 
    // create the connection with kv server
    kinetic::KineticConnectionFactory kcf = 
      kinetic::NewKineticConnectionFactory();
    std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
    //NBC* nbc;
    if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
      std::cout << "Unable to connect to kv server\n";
      return -1;
    }

    KVBencher bencher(nbc, "");    
    bencher.init_write(count, minorder, maxorder, do_cleanup);

  } else if (prefix == "sequential") {
    std::cout << "Not supported yet\n";
  } else if (prefix == "random") {
    std::cout << "Not supported yet\n";
  } else if (prefix == "cleanup") {
    std::cout << "Not supported yet\n";
  } else {
    std::cout << "Invalid prefix. "
      << "Should be init/sequential/random/cleanup.\n"
    return -1;
  }

  return 0;
}
