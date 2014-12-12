#include <stdio.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "kinetic/kinetic.h"
#include "glog/logging.h"

#include <sys/time.h>

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

int main(int argc, char* argv[])
{
  if (argc < 5) {
    std::cout << "usage: " << argv[0] << " <host> <port>" 
      << " <size> <count>" << std::endl;
    return -1;
  }

  const char* host = argv[1];
  int port = atoi(argv[2]);
  int vsize = atoi(argv[3]);
  int kvcount = atoi(argv[4]);

  kinetic::ConnectionOptions options;
  options.host = host;
  options.port = port;
  options.user_id = 1;
  options.hmac_key = "asdfasdf";

  // create the connection with kv server
  kinetic::KineticConnectionFactory kcf = 
    kinetic::NewKineticConnectionFactory();
  std::shared_ptr<kinetic::NonblockingKineticConnection> nbc;
  if (!kcf.NewNonblockingConnection(options, nbc).ok()) {
    printf("Unable to connect to kv server\n");
    return -1;
  }

  char key_buffer[100];
  int remaining = 0;
  fd_set read_fds, write_fds;
  int num_fds = 0;
  auto callback = make_shared<PutCallback>(&remaining);

  // value of the put ops
  std::string value(vsize, 'a');
  auto record = make_shared<KineticRecord>(value, "", "", 
      Message_Algorithm_SHA1);

  // record the time
  timeval begint, endt;
  double ti_sec;
  gettimeofday(&begint, NULL);

  // put to kv server
  for (int64_t i = 0; i < (int64_t)kvcount; i++) {
    // key of the put ops
    sprintf(key_buffer, "%s-%10" PRId64, "key", i);
    std::string key(key_buffer);
    remaining++;
    nbc->Put(key, "", kinetic::WriteMode::IGNORE_VERSION, record,
        callback, kinetic::PersistMode::WRITE_BACK);
    nbc->Run(&read_fds, &write_fds, &num_fds);
  }

  // wait for the ops to finish
  while (remaining > 0) {
    while (select(num_fds + 1, &read_fds, &write_fds, NULL, NULL) <= 0);
    nbc->Run(&read_fds, &write_fds, &num_fds);
  }

  gettimeofday(&endt, NULL);
  ti_sec = endt.tv_sec - begint.tv_sec;
  ti_sec += (endt.tv_usec - begint.tv_usec) / (1000.0 * 1000.0);
  std::cout << std::endl << "Done!" << std::endl;
  std::cout << "Total time is " << ti_sec << "seconds " << std::endl;
  return 0;
}
