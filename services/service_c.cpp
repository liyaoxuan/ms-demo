// service_c.cpp
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using demo::RequestC;
using demo::ResponseC;
using demo::ServiceC;

class ServiceCImpl final : public ServiceC::Service {
 public:
  Status Compute(ServerContext* context, const RequestC* request,
                                     ResponseC* response) override {
    
    // 调用服务 C
    int duration = request->time_c();
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::milliseconds(duration);

    // 定义一个volatile变量，用来阻止优化
    volatile int dummy = 0;

    // 循环直到时间到达设定的时间点
    while (std::chrono::steady_clock::now() < end) {
        dummy++;  // 增加一个无意义的计算操作
    }
    response->set_success(true);
    return Status::OK;
  }
};

void RunServer(const std::string& server_addr) {
  ServiceCImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Service C listening on " << server_addr << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] 
              << " <service_c_port>\n"
              << "Example: " << argv[0] 
              << " 50050" << std::endl;
    return 1;
  }

  const std::string server_addr = "0.0.0.0:" + std::string(argv[1]);
 
  try {
    RunServer(server_addr);
  } catch (const std::exception& e) {
    std::cerr << "Server exception: " << e.what()  << std::endl;
    return 1;
  }
  return 0;
}