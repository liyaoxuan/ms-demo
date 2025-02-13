// service_a.cpp  
#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "service.grpc.pb.h" 
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using demo::RequestA;
using demo::ResponseA;
using demo::ServiceA;
 
class ServiceAImpl final : public ServiceA::Service {
 public:
  // 添加构造函数接收下游服务地址 
  explicit ServiceAImpl(std::string downstream_addr)
      : downstream_addr_(std::move(downstream_addr)) {}
 
  Status Compute(ServerContext* context, const RequestA* request,
                 ResponseA* response) override {
    
    // 原有计算逻辑保持不变 
    int duration = request->time_a();
    auto start = std::chrono::steady_clock::now();
    auto end = start + std::chrono::milliseconds(duration);
    volatile int dummy = 0;
    while (std::chrono::steady_clock::now() < end) {
        dummy++;
    }
    response->set_success(true);
 
    // 使用传入的下游服务地址 
    auto channel = grpc::CreateChannel(downstream_addr_, grpc::InsecureChannelCredentials());
    std::unique_ptr<demo::ServiceB::Stub> stub = demo::ServiceB::NewStub(channel);
 
    demo::RequestB b_request;
    b_request.set_time_b(request->time_b()); 
    b_request.set_time_c(request->time_c()); 
 
    demo::ResponseB b_response;
    grpc::ClientContext b_context;
    Status status = stub->Compute(&b_context, b_request, &b_response);
 
    if (status.ok())  {
      std::cout << "Service B returned success: " << b_response.success()  << std::endl;
    } else {
      std::cerr << "Service B call failed: " << status.error_message()  << std::endl;
    }
 
    return Status::OK;
  }
 
 private:
  std::string downstream_addr_;  // 存储下游服务地址 
};
 
void RunServer(const std::string& server_addr, 
              const std::string& downstream_addr) {
  ServiceAImpl service(downstream_addr);
  
  ServerBuilder builder;
  builder.AddListeningPort(server_addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
 
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Service A listening on " << server_addr << std::endl;
  server->Wait();
}
 
int main(int argc, char** argv) {
  // 参数校验和帮助信息 
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] 
              << " <service_a_port> <service_b_address:port>\n"
              << "Example: " << argv[0] 
              << " 50050 svc_b:50050" << std::endl;
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