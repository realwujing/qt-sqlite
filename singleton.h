#pragma once

/**
 * @brief Singleton class
 *
 * @tparam T Type of the singleton
 *
 */
template<typename T>
class Singleton
{
public:
    static T* instance()
    {
        static T instance;
        return &instance;
    }

    Singleton(T&&) = delete;
    Singleton(const T&) = delete;
    void operator= (const T&) = delete;

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};