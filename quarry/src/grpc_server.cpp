#include "aggregates.h"
#include "base_endpoint.h"
#include "polygon.h"
#include "sql.h"
#include "utils.h"
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include "aggregates.grpc.pb.h"

namespace {
using Aggregates = quarry::ep::Aggregates;
using AggBar = quarry::ep::AggBar;

class AggregatesServiceImpl final : public marble::AggregatesService::Service {

public:
  grpc::Status GetAggregate(grpc::ServerContext *context,
                            const marble::AggregatesRequest *request,
                            marble::AggregatesResponse *response) override {

    if (request->ticker().empty()) {
      return {grpc::StatusCode::INVALID_ARGUMENT, "ticker is required"};
    }
    if (request->from_date().empty() || request->to_date().empty()) {
      return {grpc::StatusCode::INVALID_ARGUMENT,
              "from_date and to_date are required"};
    }

    try {
      auto aggregate_ep =
          Aggregates::with_ticker(request->ticker())
              .time_span(quarry::timespan_resolver(request->time_span()))
              .from_date(request->from_date())
              .to_date(request->to_date());

      std::string last_request_id;
      std::string last_ticker;

      std::lock_guard<std::mutex> guard(m_polygon_mutex);
      for (const auto &aggregate_bar_batch :
           m_polygon.execute_with_pagination(aggregate_ep)) {

        if (!aggregate_bar_batch.results.has_value() ||
            aggregate_bar_batch.results->empty()) {
          continue;
        }

        const auto &bar_batch = *aggregate_bar_batch.results;
        for (const auto &bar : bar_batch) {
          auto *proto_bar = response->add_aggregate_bars();
          proto_bar->set_open(bar.o);
          proto_bar->set_close(bar.c);
          proto_bar->set_high(bar.h);
          proto_bar->set_low(bar.l);
          proto_bar->set_n(bar.n);
          proto_bar->set_otc(bar.otc);
          proto_bar->set_t(bar.t);
          proto_bar->set_volume(bar.v);
          proto_bar->set_volume_weighted(bar.vw);
        }

        last_ticker = aggregate_bar_batch.ticker;
        last_request_id = aggregate_bar_batch.request_id;
      }

      response->set_ticker(last_ticker);
      response->set_query_count(-1); //@todo
      response->set_request_id(last_request_id);
      response->set_results_count(-1);
      response->set_count(-1);
      response->set_status("ok");
      return grpc::Status::OK;
    } catch (const std::invalid_argument &ex) {
      return {grpc::StatusCode::INVALID_ARGUMENT, ex.what()};
    } catch (const std::exception &ex) {
      return {grpc::StatusCode::INTERNAL, ex.what()};
    }
  }
  AggregatesServiceImpl(quarry::Polygon &polygon) : m_polygon(polygon) {}

private:
  quarry::Polygon &m_polygon;
  std::mutex m_polygon_mutex;
};

} // namespace

int main(int argc, char **argv) {
  // polygon setup

  quarry::load_dotenv("./quarry/.env");
  const char *api_key = std::getenv("POLYGON_API_KEY");
  if (api_key == nullptr || std::string_view(api_key).empty()) {
    std::cerr << "POLYGON_API_KEY missing; refusing to start server"
              << std::endl;
    return 1;
  }
  quarry::Polygon polygon(api_key);

  // grpc setup
  std::string server_address = "0.0.0.0:50051";
  if (argc > 1) {
    server_address = argv[1];
  }

  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  AggregatesServiceImpl service{polygon};
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "gRPC server listening on " << server_address << std::endl;
  server->Wait();
  return 0;
}
