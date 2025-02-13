// service_b.cpp  
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "service.grpc.pb.h" 
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using demo::RequestB;
using demo::ResponseB;
using demo::ServiceB;
 
class ServiceBImpl final : public ServiceB::Service {
 public:
  // 新增构造函数接收下游地址 
  explicit ServiceBImpl(std::string downstream_addr)
      : downstream_addr_(std::move(downstream_addr)) {}
 
  Status Compute(ServerContext* context, const RequestB* request,
                 ResponseB* response) override {
    // 原有计算逻辑保持不变 
    int duration = request->time_b();
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::milliseconds(duration);
    volatile int dummy = 0;
    while (std::chrono::steady_clock::now() < end) {
        dummy++;
    }
    response->set_success(true);
 
    // 使用动态配置的下游地址 
    auto channel = grpc::CreateChannel(downstream_addr_, grpc::InsecureChannelCredentials());
    std::unique_ptr<demo::ServiceC::Stub> stub = demo::ServiceC::NewStub(channel);
 
    demo::RequestC c_request;
    c_request.set_time_c(request->time_c()); 
 
    demo::ResponseC c_response;
    grpc::ClientContext c_context;
    Status status = stub->Compute(&c_context, c_request, &c_response);
 
    if (status.ok())  {
      std::cout << "Service C returned success: " << c_response.success()  << std::endl;
    } else {
      std::cerr << "Service C call failed: " << status.error_message()  << std::endl;
    }
 
    return Status::OK;
  }
 
 private:
  std::string downstream_addr_;  // 存储下游服务地址 
};
 
void RunServer(const std::string& server_addr, 
              const std::string& downstream_addr) {
  ServiceBImpl service(downstream_addr);
 
  ServerBuilder builder;
  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
 
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Service B listening on " << server_addr << std::endl;
  server->Wait();
}
 
int main(int argc, char** argv) {
  // 参数校验和帮助信息 
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] 
              << " <service_b_port> <service_c_address:port>\n"
              << "Example: " << argv[0] 
              << " 50050 svc_c:50050" << std::endl;
    return 1;
  }

  const std::string server_addr = "0.0.0.0:" + std::string(argv[1]);
  const std::string downstream_addr = argv[2];
 
  try {
    RunServer(server_addr, downstream_addr);
  } catch (const std::exception& e) {
    std::cerr << "Server exception: " << e.what()  << std::endl;
    return 1;
  }
  return 0;
}