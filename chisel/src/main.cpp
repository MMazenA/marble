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

  void do_unary(marble::AggregatesResponse &response, const std::string &ticker,
                const std::string &from_date, const std::string &to_date,
                marble::timespan_options time_span) {
    marble::AggregatesRequest request;

    // request args
    request.set_ticker(ticker);
    request.set_from_date(from_date);
    request.set_to_date(to_date);
    request.set_time_span(time_span);

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

    for (const auto &x : response.aggregate_bars()) {
      std::cout << "does it work: " << x.open() << std::endl;
    }
  }

private:
  std::unique_ptr<marble::AggregatesService::Stub> stub_;
};

} // namespace

int main(int argc, char **argv) {
  std::string target = "localhost:50051";
  AggregatesClient client(
      grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

  marble::AggregatesResponse response;
  client.do_unary(response, "AAPL", "2024-01-01", "2024-01-09",
                  marble::timespan_options::DAY);

  return 0;
}
