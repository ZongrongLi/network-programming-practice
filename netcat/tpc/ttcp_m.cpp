#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <iostream>

#include <boost/program_options.hpp>

#include <sys/time.h>

struct Options
{
  uint16_t port;
  int length;
  int number;
  bool transmit, receive, nodelay;
  std::string host;
  Options()
    : port(0), length(0), number(0),
      transmit(false), receive(false), nodelay(false)
  {
  }
};

struct SessionMessage
{
  int32_t number;
  int32_t length;
} __attribute__ ((__packed__));

struct PayloadMessage
{
  int32_t length;
  char data[0];
};

double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// FIXME: rewrite with getopt(3).
bool parseCommandLine(int argc, char* argv[], Options* opt)
{
  namespace po = boost::program_options;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help,h", "Help")
      ("port,p", po::value<uint16_t>(&opt->port)->default_value(5001), "TCP port")
      ("length,l", po::value<int>(&opt->length)->default_value(65536), "Buffer length")
      ("number,n", po::value<int>(&opt->number)->default_value(8192), "Number of buffers")
      ("trans,t",  po::value<std::string>(&opt->host), "Transmit")
      ("recv,r", "Receive")
      ("nodelay,D", "set TCP_NODELAY")
      ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  opt->transmit = vm.count("trans");
  opt->receive = vm.count("recv");
  opt->nodelay = vm.count("nodelay");
  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return false;
  }

  if (opt->transmit == opt->receive)
  {
    printf("either -t or -r must be specified.\n");
    return false;
  }

  printf("port = %d\n", opt->port);
  if (opt->transmit)
  {
    printf("buffer length = %d\n", opt->length);
    printf("number of buffers = %d\n", opt->number);
  }
  else
  {
    printf("accepting...\n");
  }
  return true;
}


void receive(const Options& opt){
    Acceptor ap(InetAddress(opt.port));
    TcpStreamPtr stream(ap.accept());
    assert(stream!=NULL);

    struct SessionMessage sess ={0,0};
    if( stream->receiveAll(&sess,sizeof(sess)) != sizeof(sess)) {
        perror("session wrong");
        return;
    }
    sess.length = ntohl(sess.length);
    sess.number = ntohl(sess.number);
    int totlen = sizeof(int32_t) + sess.length;
    PayloadMessage *payload = static_cast<PayloadMessage*>(::malloc(totlen));
    std::unique_ptr<PayloadMessage,void(*)(void*)> freeIt(payload,::free);
    assert(payload);
    double start = now();
    for(int i=0;i<sess.number;i++){
        int nr = stream->receiveAll(payload,totlen);
        assert(nr==totlen);


        const int ack =  payload->length;//no convert of bytearray
        int ns = stream->sendAll(&ack,sizeof(ack));

        assert(ns==sizeof(ack));
    }

    double total_mb = 1.0 * opt.length * opt.number / 1024 / 1024;
    printf("%.3f MiB in total\n", total_mb);
    double elapsed = now() - start;
    printf("%.3f seconds\n%.3f MiB/s\n", elapsed, total_mb / elapsed);

}
void transmit(const Options& opt)
{
    double start = now();
    InetAddress addr(opt.port);
    if(!InetAddress::resolve(opt.host.c_str(),&addr) ){
            printf("unable to resolve %s",opt.host.c_str());
            return;
    }

    TcpStreamPtr stream(TcpStream::connect(addr));
    if(!stream){
        return ;
    }
    struct SessionMessage sess = {0,0};
    sess.length = htonl( opt.length);
    sess.number = htonl( opt.number);
    if(stream->sendAll(&sess,sizeof(sess))!=sizeof(sess) ){
          perror("write session message") ;
          return;
    }

    const int totlen = sizeof(int32_t) + opt.length;
    PayloadMessage *payload =  static_cast<PayloadMessage*>( ::malloc(totlen) );
    std::unique_ptr<PayloadMessage,void(*)(void*)> freeIt(payload,::free);
    payload->length = htonl(opt.length);
    for(int i=0;i<opt.length;i++){
        payload->data[i] = "0123456789ABCDEF"[i%16];
    }
    for(int i =0;i<opt.number;i++){
        int nw= stream->sendAll(payload,totlen);
            assert(nw == totlen);

        int ack;
        int nr = stream->receiveAll(&ack,sizeof(ack));
        assert(nr== sizeof(ack));
        ack = ntohl(ack);
        assert(ack== opt.length);
    }
    std::cout<<"hehe"<<std::endl;
    double total_mb = 1.0 * opt.length * opt.number / 1024 / 1024;
    printf("%.3f MiB in total\n", total_mb);
    double elapsed = now() - start;
    printf("%.3f seconds\n%.3f MiB/s\n", elapsed, total_mb / elapsed);

}

int main(int argc, char* argv[])
{
  Options options;
  if (parseCommandLine(argc, argv, &options))
  {
    if (options.transmit)
    {
     // transmit(options);
    }
    else if (options.receive)
    {
      receive(options);
    }
    else
    {
      assert(0);
    }
  }
}
