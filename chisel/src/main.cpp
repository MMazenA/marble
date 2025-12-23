#include <iostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "aggregates.grpc.pb.h"

namespace {

class AggregatesClient {
public:
  explicit AggregatesClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(marble::AggregatesService::NewStub(std::move(channel))) {}

  void do_unary() {
    marble::AggregatesRequest request;
    marble::AggregatesResponse response;
    grpc::ClientContext context;
    const grpc::Status status =
        stub_->GetAggregate(&context, request, &response);

    if (!status.ok()) {
      std::cerr << "Unary RPC failed: " << status.error_message() << std::endl;
      return;
    }

    std::cout << "response ticker=" << response.ticker()
              << " request id=" << response.request_id()
              << " status= " << response.status() << std::endl;
  }

private:
  std::unique_ptr<marble::AggregatesService::Stub> stub_;
};

} // namespace

int main(int argc, char **argv) {
  std::string target = "localhost:50051";
  if (argc > 1) {
    target = argv[1];
  }

  AggregatesClient client(
      grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
  client.do_unary();

  return 0;
}
