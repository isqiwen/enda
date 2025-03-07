#pragma once

#include "Macros.hpp"

#include <sstream>
#include <string>

#define ENDA_RUNTIME_ERROR throw enda::runtime_error {} << "Error at " << __FILE__ << " : " << __LINE__ << "\n\n"

#define ENDA_ASSERT(X) \
    if (!(X)) \
        ENDA_RUNTIME_ERROR << AS_STRING(X);

#define ENDA_ASSERT2(X, ...) \
    if (!(X)) \
        ENDA_RUNTIME_ERROR << AS_STRING(X) << "\n" << __VA_ARGS__;

namespace enda
{
    class runtime_error : public std::exception
    {
        // Accumulator for the error message.
        std::stringstream acc;

        // Error message.
        mutable std::string _what;

    public:
        /// Default constructor.
        runtime_error() noexcept : std::exception() {}

        /**
         * @brief Copy constructor to copy the contents of the error message accumulator.
         * @param err Runtime error to copy.
         */
        runtime_error(runtime_error const& err) noexcept : acc(err.acc.str()), _what(err._what) {}

        /// Default destructor.
        ~runtime_error() noexcept override = default;

        /**
         * @brief Accumulate error message.
         *
         * @tparam T Type to accumulate.
         * @param x Input value to accumulate.
         * @return Reference to `this` with the input value appended to the error message.
         */
        template<typename T>
        runtime_error& operator<<(T const& x)
        {
            acc << x;
            return *this;
        }

        /**
         * @brief Accumulate error message.
         *
         * @param mess Message to accumulate.
         * @return Reference to `this` with the input message appended to the error message.
         */
        runtime_error& operator<<(const char* mess)
        {
            (*this) << std::string(mess);
            return *this;
        }

        /**
         * @brief Override the virtual function `what` from std::exception to retrieve the accumulated error message.
         * @return Error message.
         */
        const char* what() const noexcept override
        {
            _what = acc.str();
            return _what.c_str();
        }
    };

} // namespace enda
