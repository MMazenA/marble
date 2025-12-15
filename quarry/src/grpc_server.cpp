#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include "aggregates.grpc.pb.h"

namespace {

class AggregatesServiceImpl final : public marble::AggregatesService::Service {
public:
  grpc::Status GetAggregate(grpc::ServerContext *context,
                            const marble::AggregatesRequest *request,
                            marble::AggregatesResponse *response) override {
    response->set_id(request->id());
    response->set_name(request->name().empty() ? "demo" : request->name());
    response->set_status("ok");
    return grpc::Status::OK;
  }

  grpc::Status WatchAggregates(
      grpc::ServerContext *context,
      const marble::AggregatesStreamRequest *request,
      grpc::ServerWriter<marble::AggregatesUpdate> *writer) override {
    const std::string filter =
        request->filter().empty() ? "all" : request->filter();

    std::vector<marble::AggregatesUpdate> updates;
    for (int i = 0; i < 3; ++i) {
      marble::AggregatesUpdate update;
      update.set_id(i + 1);
      update.set_name("aggregate-" + std::to_string(i + 1));
      update.set_note("filter=" + filter + " seq=" + std::to_string(i + 1));
      updates.push_back(update);
    }

    for (const auto &update : updates) {
      writer->Write(update);
      std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    return grpc::Status::OK;
  }

  grpc::Status
  Chat(grpc::ServerContext *context,
       grpc::ServerReaderWriter<marble::StreamMessage, marble::StreamMessage>
           *stream) override {
    marble::StreamMessage incoming;
    while (stream->Read(&incoming)) {
      marble::StreamMessage reply;
      reply.set_id(incoming.id());
      reply.set_payload("echo: " + incoming.payload());
      stream->Write(reply);
    }
    return grpc::Status::OK;
  }
};

} // namespace

int main(int argc, char **argv) {
  std::string server_address = "0.0.0.0:50051";
  if (argc > 1) {
    server_address = argv[1];
  }

  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  AggregatesServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "gRPC server listening on " << server_address << std::endl;
  server->Wait();
  return 0;
}
