#include <iostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "aggregates.grpc.pb.h"

namespace {

// Simple demo client that exercises unary, server streaming, and bidi
// streaming.
class AggregatesClient {
public:
  explicit AggregatesClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(marble::AggregatesService::NewStub(std::move(channel))) {}

  void do_unary() {
    marble::AggregatesRequest request;
    request.set_id(42);
    request.set_name("sample");

    marble::AggregatesResponse response;
    grpc::ClientContext context;
    const grpc::Status status =
        stub_->GetAggregate(&context, request, &response);

    if (!status.ok()) {
      std::cerr << "Unary RPC failed: " << status.error_message() << std::endl;
      return;
    }

    std::cout << "Unary response id=" << response.id()
              << " name=" << response.name() << " status=" << response.status()
              << std::endl;
  }

  void do_server_streaming() {
    marble::AggregatesStreamRequest request;
    request.set_filter("demo");

    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<marble::AggregatesUpdate>> reader(
        stub_->WatchAggregates(&context, request));

    marble::AggregatesUpdate update;
    while (reader->Read(&update)) {
      std::cout << "Update id=" << update.id() << " name=" << update.name()
                << " note=" << update.note() << std::endl;
    }

    const grpc::Status status = reader->Finish();
    if (!status.ok()) {
      std::cerr << "Streaming RPC failed: " << status.error_message()
                << std::endl;
    }
  }

  void do_bidi_streaming() {
    grpc::ClientContext context;
    std::shared_ptr<
        grpc::ClientReaderWriter<marble::StreamMessage, marble::StreamMessage>>
        stream(stub_->Chat(&context));

    std::vector<std::string> payloads = {"one", "two", "three"};

    for (const auto &payload : payloads) {
      marble::StreamMessage outgoing;
      outgoing.set_id(static_cast<int>(payload.size()));
      outgoing.set_payload(payload);

      if (!stream->Write(outgoing)) {
        std::cerr << "Stream closed while writing" << std::endl;
        break;
      }

      marble::StreamMessage incoming;
      if (stream->Read(&incoming)) {
        std::cout << "Chat echo id=" << incoming.id()
                  << " payload=" << incoming.payload() << std::endl;
      } else {
        std::cerr << "Stream closed before echo" << std::endl;
        break;
      }
    }

    stream->WritesDone();
    const grpc::Status status = stream->Finish();
    if (!status.ok()) {
      std::cerr << "Bidi streaming RPC failed: " << status.error_message()
                << std::endl;
    }
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
  client.do_server_streaming();
  client.do_bidi_streaming();
  return 0;
}
