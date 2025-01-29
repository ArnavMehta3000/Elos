#pragma once

#include <type_traits>

#define ELOS_ENUM_FLAGS(Enum)                                              \
constexpr Enum operator|(Enum lhs, Enum rhs) noexcept                      \
{                                                                          \
    using UT = std::underlying_type_t<Enum>;                               \
    return static_cast<Enum>(static_cast<UT>(lhs) | static_cast<UT>(rhs)); \
}                                                                          \
constexpr Enum operator&(Enum lhs, Enum rhs) noexcept                      \
{                                                                          \
    using UT = std::underlying_type_t<Enum>;                               \
    return static_cast<Enum>(static_cast<UT>(lhs) & static_cast<UT>(rhs)); \
}                                                                          \
constexpr Enum operator^(Enum lhs, Enum rhs) noexcept                      \
{                                                                          \
    using UT = std::underlying_type_t<Enum>;                               \
    return static_cast<Enum>(static_cast<UT>(lhs) ^ static_cast<UT>(rhs)); \
}                                                                          \
constexpr Enum operator~(Enum e) noexcept                                  \
{                                                                          \
    using UT = std::underlying_type_t<Enum>;                               \
    return static_cast<Enum>(~static_cast<UT>(e));                         \
}                                                                          \
constexpr Enum& operator|=(Enum& lhs, Enum rhs) noexcept                   \
{                                                                          \
    return lhs = (lhs | rhs);                                              \
}                                                                          \
constexpr Enum& operator&=(Enum& lhs, Enum rhs) noexcept                   \
{                                                                          \
    return lhs = (lhs & rhs);                                              \
}                                                                          \
constexpr Enum& operator^=(Enum& lhs, Enum rhs) noexcept                   \
{                                                                          \
    return lhs = (lhs ^ rhs);                                              \
}