#include <chrono>
#include <cstdlib>
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

  void do_aggregate(marble::AggregatesResponse &response,
                    const std::string &ticker, const std::string &from_date,
                    const std::string &to_date,
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

    // std::cout << "response ticker=" << response.ticker()
    //           << " request id=" << response.request_id()
    //           << " status= " << response.status() << std::endl;

    // for (const auto &x : response.aggregate_bars()) {
    //   std::cout << "does it work: " << x.open() << std::endl;
    // }
  }

private:
  std::unique_ptr<marble::AggregatesService::Stub> stub_;
};

} // namespace

int main(int argc, char **argv) {
  using namespace std::chrono;
  std::string target = "localhost:50051";
  int iterations = 1;
  if (argc > 1) {
    iterations = std::max(1, std::atoi(argv[1]));
  }

  const auto channel_start = steady_clock::now();
  auto channel =
      grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
  const auto channel_ready = steady_clock::now();

  AggregatesClient client(channel);
  const auto client_ready = steady_clock::now();

  auto print_ms = [](auto start, auto end, const char *label) {
    const auto ms = duration_cast<microseconds>(end - start).count() / 1000.0;
    std::cout << label << ": " << ms << " ms" << std::endl;
  };

  print_ms(channel_start, channel_ready, "channel setup");
  print_ms(channel_ready, client_ready, "stub setup");

  for (int i = 0; i < iterations; ++i) {
    marble::AggregatesResponse response;
    const auto rpc_start = steady_clock::now();
    client.do_aggregate(response, "AAPL", "2024-01-01", "2024-01-09",
                        marble::timespan_options::DAY);
    const auto rpc_end = steady_clock::now();
    std::cout << "rpc[" << i << "] duration: "
              << duration_cast<microseconds>(rpc_end - rpc_start).count() /
                     1000.0
              << " ms" << std::endl;
  }

  return 0;
}
