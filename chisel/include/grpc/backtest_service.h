#ifndef CHISEL_GRPC_BACKTEST_SERVICE_H
#define CHISEL_GRPC_BACKTEST_SERVICE_H

#include <functional>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "back_test.grpc.pb.h"
#include "back_test.pb.h"
#include "engine/engine.h"

namespace chisel {

using BarProvider =
    std::function<std::vector<Bar>(const marble::BacktestRequest &)>;

class BacktestServiceImpl final : public marble::BacktestService::Service {
public:
  explicit BacktestServiceImpl(BarProvider provider)
      : m_provider(std::move(provider)) {}

  grpc::Status
  RunBackTest(grpc::ServerContext *ctx, const marble::BacktestRequest *req,
              grpc::ServerWriter<marble::BacktestEvent> *writer) override;

private:
  BarProvider m_provider;
};

} // namespace chisel

#endif // CHISEL_GRPC_BACKTEST_SERVICE_H
