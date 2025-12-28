#ifndef QUARRY_LOGGING_H
#define QUARRY_LOGGING_H

#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/LogMacros.h>
#include <quill/core/Common.h>
#include <quill/core/LogLevel.h>
#include <quill/sinks/ConsoleSink.h>
#include <quill/sinks/RotatingFileSink.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace quarry::logging {
static constexpr std::size_t mi_b = 1024ZU * 1024ZU;

struct HighThroughputFrontendOptions {
  // drop logs to preserve speed
  static constexpr quill::QueueType queue_type =
      quill::QueueType::UnboundedDropping;
  // per thread capacities
  static constexpr std::size_t unbounded_queue_max_capacity = 512 * mi_b;
  static constexpr size_t initial_queue_capacity = 256 * mi_b;
  // not doing anything with queuetype since non-blocking
  static constexpr uint32_t blocking_queue_retry_interval_ns = 200;
  // linux only
  static constexpr quill::HugePagesPolicy huge_pages_policy =
      quill::HugePagesPolicy::Never;
};

using Logger = quill::LoggerImpl<HighThroughputFrontendOptions>;
using Frontend = quill::FrontendImpl<HighThroughputFrontendOptions>;

struct Config {
  std::string logger_name = "quarry";
  std::string log_path = "logs/quarry.log";
  bool console = true;
  size_t rotation_max_file_size = 256 * mi_b;
  uint32_t max_backup_files = 4;
  size_t write_buffer_size = 1 * mi_b;
  bool fsync = false;
  char open_mode = 'a';
  std::chrono::nanoseconds backend_sleep = std::chrono::nanoseconds{0};
  bool backend_yield_when_idle = true;
  std::optional<quill::LogLevel> log_level = std::nullopt;
};

inline Logger *init(Config config = {}) {
  static std::once_flag init_once;
  static Logger *logger = nullptr;

  std::call_once(init_once, [&]() {
    if (!config.log_path.empty()) {
      std::filesystem::path log_path{config.log_path};
      if (log_path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(log_path.parent_path(), ec);
      }
    }

    quill::BackendOptions backend_options;
    backend_options.sleep_duration = config.backend_sleep;
    backend_options.enable_yield_when_idle = config.backend_yield_when_idle;
    quill::Backend::start(backend_options);

    Frontend::preallocate();

    quill::RotatingFileSinkConfig file_config;
    file_config.set_rotation_max_file_size(config.rotation_max_file_size);
    file_config.set_max_backup_files(config.max_backup_files);
    file_config.set_fsync_enabled(config.fsync);
    file_config.set_write_buffer_size(config.write_buffer_size);
    file_config.set_open_mode(config.open_mode);

    auto file_sink = Frontend::create_or_get_sink<quill::RotatingFileSink>(
        config.log_path, file_config);

    std::vector<std::shared_ptr<quill::Sink>> sinks;
    sinks.emplace_back(file_sink);

    if (config.console) {
      quill::ConsoleSinkConfig console_config;
      console_config.set_stream("stderr");
      auto console_sink = Frontend::create_or_get_sink<quill::ConsoleSink>(
          "quarry_console_sink", console_config);
      sinks.emplace_back(console_sink);
    }

    logger = Frontend::create_or_get_logger(config.logger_name, sinks);
    if (config.log_level.has_value()) {
      logger->set_log_level(*config.log_level);
    }
  });

  return logger;
}

inline Logger *get_logger() { return init(); }

inline void preallocate_thread_context() { Frontend::preallocate(); }

} // namespace quarry::logging

#endif
