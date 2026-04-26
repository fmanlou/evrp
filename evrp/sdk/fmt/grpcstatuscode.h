#pragma once

#include <grpcpp/support/status_code_enum.h>

#include <fmt/format.h>

#include <string_view>

std::string_view toString(grpc::StatusCode code);

template <>
struct fmt::formatter<grpc::StatusCode> {
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(grpc::StatusCode c, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    const std::string_view label = toString(c);
    if (!label.empty()) {
      return fmt::format_to(ctx.out(), "{}", label);
    }
    return fmt::format_to(ctx.out(), "StatusCode({})", static_cast<int>(c));
  }
};
