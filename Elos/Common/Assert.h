#pragma once

#include <Elos/Common/FunctionMacros.h>
#include <Elos/Common/String.h>
#include <source_location>
#include <format>
#include <stdexcept>
#include <typeinfo>
#include <stacktrace>

namespace Elos
{
    namespace Internal
    {
        template<typename T>
        concept Stringable = requires(T t) { std::format("{}", t); };

        template<typename T>
        NODISCARD StringView GetTypeName() noexcept
        {
            return typeid(T).name();
        }

        // Helper function to extract filename from full path
        NODISCARD String GetFileName(StringView path) noexcept
        {
            size_t lastSlash = path.find_last_of("\\/");
            return String(lastSlash != StringView::npos ? path.substr(lastSlash + 1) : path);
        }

        // Helper function to format stacktrace
        NODISCARD String FormatStackTrace(const std::stacktrace& trace) noexcept
        {
            // Skip topmost 2 frames since they are part of assert formatters
            constexpr size_t startFrame = 2;
            // Skip last 6 frame since they are part of getting to the main function
            const size_t endFrame = trace.size() - 6;

            // Remove the addresses
            const auto FormatDesc = [](const std::string& desc)
            {
                const size_t index = desc.find_last_of('+');
                return desc.substr(0, index);
            };

            // Format only the frames we want to keep
            String result;
            for (size_t i = startFrame; i < endFrame; ++i)
            {
                const auto& frame = trace[i];
                String filename = GetFileName(frame.source_file());

                result += std::format("\t{}({}) - {}\n",
                    frame.source_file(),
                    frame.source_line(),
                    FormatDesc(frame.description()));
            }

            return result;
        }

        // Helper function to format common assertion info
        NODISCARD String FormatAssertionLocation(const std::source_location& location) noexcept
        {
            return std::format(
                "{}({})\n"
                "Function: {}\n",
                location.file_name(),
                location.line(),
                location.function_name()
            );
        }
    }

    class AssertionFailure : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
        explicit AssertionFailure(const String& message) : std::runtime_error(message) {}
        explicit AssertionFailure(const char* message) : std::runtime_error(message) {}
    };

    template<typename Expr>
    class AssertBuilder
    {
    public:
        AssertBuilder(Expr&& expr, const std::source_location& location = std::source_location::current())
            : m_expr(std::forward<Expr>(expr))
            , m_location(location)
            , m_includeCallStack(true)
        {
        }

        // Add custom message
        template<typename... Args>
        NODISCARD AssertBuilder& Msg(std::format_string<Args...> fmt, Args&&... args)
        {
            m_message = std::format(fmt, std::forward<Args>(args)...);
            return *this;
        }

        // Disable call stack in output
        NODISCARD AssertBuilder& NoCallStack()
        {
            m_includeCallStack = false;
            return *this;
        }

        // Dump the assertion message without throwing
        NODISCARD String Dump() const
        {
            if (!static_cast<bool>(m_expr))
            {
                return FormatErrorMessage();
            }
            return "";
        }

        // Throw an exception if assertion fails
        void Throw() const
        {
            if (!static_cast<bool>(m_expr))
            {
                throw AssertionFailure(FormatErrorMessage());
            }
        }

        operator bool() const
        {
            if (!static_cast<bool>(m_expr))
            {
                throw AssertionFailure(FormatErrorMessage());
            }
            return true;
        }

    private:
        NODISCARD String FormatErrorMessage() const
        {
            String error_message = std::format(
                "\nAssertion failed!\n{}Type: {}\n",
                Internal::FormatAssertionLocation(m_location),
                Internal::GetTypeName<std::remove_cvref_t<Expr>>()
            );

            if (!m_message.empty())
            {
                error_message += std::format("Message: {}\n", m_message);
            }

            if (m_includeCallStack)
            {
                error_message += "\nCall stack:\n";
                error_message += Internal::FormatStackTrace(std::stacktrace::current());
            }

            return error_message;
        }


    private:
        Expr                 m_expr;
        std::source_location m_location;
        String               m_message;
        bool                 m_includeCallStack;
    };

    // Helper function to deduce template parameters
    template<typename Expr>
    AssertBuilder<Expr> ASSERT(Expr&& expr, const std::source_location& location = std::source_location::current())
    {
        return AssertBuilder<Expr>(std::forward<Expr>(expr), location);
    }

    template<typename T, typename U>
    class AssertEqualityBuilder
    {
    public:
        AssertEqualityBuilder(const T& actual, const U& expected, const std::source_location& location = std::source_location::current())
            : m_actual(actual)
            , m_expected(expected)
            , m_location(location)
            , m_includeCallStack(true)
        {
        }

        // Add custom message
        template<typename... Args>
        NODISCARD AssertEqualityBuilder& Msg(std::format_string<Args...> fmt, Args&&... args)
        {
            m_message = std::format(fmt, std::forward<Args>(args)...);
            return *this;
        }

        // Disable call stack in output
        NODISCARD AssertEqualityBuilder& NoCallStack()
        {
            m_includeCallStack = false;
            return *this;
        }

        // Dump the assertion message without throwing
        NODISCARD String Dump() const
        {
            if (!(m_actual == m_expected))
            {
                return FormatErrorMessage();
            }
            return "";
        }

        // Throw an exception if assertion fails
        void Throw() const
        {
            if (!(m_actual == m_expected))
            {
                throw AssertionFailure(FormatErrorMessage());
            }
        }

        operator bool() const
        {
            if (!(m_actual == m_expected))
            {
                throw AssertionFailure(FormatErrorMessage());
            }
            return true;
        }

    private:
        NODISCARD String FormatErrorMessage() const
        {
            String error_message = std::format(
                "\nEquality assertion failed!\n"
                "{}Types: {} vs {}\n"
                "Expected: {}\n"
                "Actual: {}\n",
                Internal::FormatAssertionLocation(m_location),
                Internal::GetTypeName<std::remove_cvref_t<T>>(),
                Internal::GetTypeName<std::remove_cvref_t<U>>(),
                m_expected,
                m_actual
            );

            if (!m_message.empty())
            {
                error_message += std::format("Message: {}\n", m_message);
            }

            if (m_includeCallStack)
            {
                error_message += "\nCall stack:\n";
                error_message += Internal::FormatStackTrace(std::stacktrace::current());
            }

            return error_message;
        }

    private:
        const T& m_actual;
        const U& m_expected;
        std::source_location m_location;
        String m_message;
        bool m_includeCallStack;
    };

    template<typename T, typename U> requires Internal::Stringable<T>&& Internal::Stringable<U>
    AssertEqualityBuilder<T, U> ASSERT_EQ(const T& actual, const U& expected, const std::source_location& location = std::source_location::current())
    {
        return AssertEqualityBuilder<T, U>(actual, expected, location);
    }

    template<std::floating_point T, std::floating_point U>
    class AssertNearBuilder
    {
    public:
        AssertNearBuilder(const T& actual, const U& expected, const double epsilon = 1e-10, const std::source_location& location = std::source_location::current())
            : m_actual(actual)
            , m_expected(expected)
            , m_epsilon(epsilon)
            , m_location(location)
            , m_includeCallStack(true)
        {
        }

        template<typename... Args>
        NODISCARD AssertNearBuilder& Msg(std::format_string<Args...> fmt, Args&&... args)
        {
            m_message = std::format(fmt, std::forward<Args>(args)...);
            return *this;
        }

        NODISCARD AssertNearBuilder& NoCallStack()
        {
            m_includeCallStack = false;
            return *this;
        }

        NODISCARD String Dump() const
        {
            if (std::abs(m_actual - m_expected) > m_epsilon)
            {
                return FormatErrorMessage();
            }
            return "";
        }

        void Throw() const
        {
            if (std::abs(m_actual - m_expected) > m_epsilon)
            {
                throw AssertionFailure(FormatErrorMessage());
            }
        }

        operator bool() const
        {
            if (std::abs(m_actual - m_expected) > m_epsilon)
            {
                throw AssertionFailure(FormatErrorMessage());
            }
            return true;
        }

    private:
        NODISCARD String FormatErrorMessage() const
        {
            String error_message = std::format(
                "\nNear-equality assertion failed!\n"
                "{}Types: {} vs {}\n"
                "Expected: {}\n"
                "Actual: {}\n"
                "Difference: {}\n"
                "Epsilon: {}\n",
                Internal::FormatAssertionLocation(m_location),
                Internal::GetTypeName<std::remove_cvref_t<T>>(),
                Internal::GetTypeName<std::remove_cvref_t<U>>(),
                m_expected,
                m_actual,
                std::abs(m_actual - m_expected),
                m_epsilon
            );

            if (!m_message.empty())
            {
                error_message += std::format("Message: {}\n", m_message);
            }

            if (m_includeCallStack)
            {
                error_message += "\nCall stack:\n";
                error_message += Internal::FormatStackTrace(std::stacktrace::current());
            }

            return error_message;
        }

    private:
        const T& m_actual;
        const U& m_expected;
        double m_epsilon;
        std::source_location m_location;
        String m_message;
        bool m_includeCallStack;
    };

    template<std::floating_point T, std::floating_point U>
    AssertNearBuilder<T, U> ASSERT_NEAR(
        const T& actual,
        const U& expected,
        const double epsilon = 1e-10,
        const std::source_location& location = std::source_location::current())
    {
        return AssertNearBuilder<T, U>(actual, expected, epsilon, location);
    }

    template<typename Container>
    auto ASSERT_EMPTY(const Container& container)
    {
        return ASSERT(container.empty())
            .Msg("Container expected to be empty, size: {}", container.size());
    }

    template<typename T>
    auto ASSERT_NOT_NULL(const T* ptr) 
    {
        return ASSERT(ptr != nullptr)
            .Msg("Expected non-null pointer of type: {}", Internal::GetTypeName<T>());
    }

    template<typename T, typename U, typename V>
    auto ASSERT_IN_RANGE(const T& value, const U& min, const V& max) 
    {
        return ASSERT(value >= min && value <= max)
            .Msg("Value {} expected to be in range [{}, {}]", value, min, max);
    }
}
