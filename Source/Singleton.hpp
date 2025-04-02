#pragma once

#include <type_traits>

namespace enda
{
    template<typename T>
    class singleton
    {
    public:
        static T& instance() noexcept
        {
            static T instance;
            return instance;
        }

        singleton(const singleton&)            = delete;
        singleton& operator=(const singleton&) = delete;
        singleton(singleton&&)                 = delete;
        singleton& operator=(singleton&&)      = delete;

    protected:
        singleton()  = default;
        ~singleton() = default;
    };

    static_assert(std::is_empty_v<singleton<int>>);

} // namespace enda
