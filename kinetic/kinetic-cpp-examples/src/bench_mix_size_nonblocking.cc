#include <stdio.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

#include <sys/time.h>
#include "kv_bench.h"

using com::seagate::kinetic::client::proto::Message_Algorithm_SHA1;
using kinetic::Status;
using kinetic::KineticRecord;
using kinetic::PutCallbackInterface;
using kinetic::KineticStatus;

using std::make_shared;
using std::unique_ptr;

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

    // create the connection with kv server
    kinetic::KineticConnectionFactory kcf = 
      kinetic::NewKineticConnectionFactory();
    std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
    //NBC* nbc;
    if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
      printf("Unable to connect to kv server\n");
      return -1;
    }

    KVBencher bencher(nbc, "");    
    bencher.init_write(count, minorder, maxorder, do_cleanup);

  } else if (prefix == "sequential") {
    if (argc < 5) {
      std::cout << "usage: "
        << argv[0]
        << " <server_ip>"
        << " <server_port>"
        << " sequential"
        << " <write_percentage>"
        << std::endl;
      return -1;
    }
    const char* host = argv[1];
    int port = atoi(argv[2]);
    uint32_t write_per = (uint32_t)atoi(argv[4]);

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    // create the connection with kv server
    kinetic::KineticConnectionFactory kcf = 
      kinetic::NewKineticConnectionFactory();
    std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
    //NBC* nbc;
    if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
      printf("Unable to connect to kv server\n");
      return -1;
    }

    KVBencher bencher(nbc, "");    
    bencher.sequential_bench(write_per);

  } else if (prefix == "random") {
    if (argc < 7) {
      std::cout << "usage: "
        << argv[0]
        << " <server_ip>"
        << " <server_port>"
        << " random"
        << " <count>"
        << " <random_rule>"
        << " <write_percentage>"
        << std::endl;
      return -1;
    }
    const char* host = argv[1];
    int port = atoi(argv[2]);
    uint32_t count = (uint32_t)atoi(argv[4]);
    std::string random_rule(argv[5]);
    uint32_t write_per = (uint32_t)atoi(argv[6]);

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    // create the connection with kv server
    kinetic::KineticConnectionFactory kcf = 
      kinetic::NewKineticConnectionFactory();
    std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
    //NBC* nbc;
    if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
      printf("Unable to connect to kv server\n");
      return -1;
    }

    KVBencher bencher(nbc, "");    
    bencher.random_bench(count, random_rule, write_per);

  } else if (prefix == "cleanup") {
    const char* host = argv[1];
    int port = atoi(argv[2]);

    kinetic::ConnectionOptions options;
    options.host = host;
    options.port = port;
    options.user_id = 1;
    options.hmac_key = "asdfasdf";

    // create the connection with kv server
    kinetic::KineticConnectionFactory kcf = 
      kinetic::NewKineticConnectionFactory();
    std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
    //NBC* nbc;
    if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
      printf("Unable to connect to kv server\n");
      return -1;
    }
    KVBencher bencher(nbc, "");
    bencher.cleanup();

  } else {
    std::cout << "Invalid prefix. Should be init/sequential/random/cleanup."
      << std::endl;
    return -1;
  }

  return 0;
}
