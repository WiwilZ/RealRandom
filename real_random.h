/*
 * Created by WiwilZ on 2022/6/9.
 */

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <wincrypt.h>

#else

#include <fcntl.h>
#include <unistd.h>

#endif

#include <memory_resource>


namespace {
    class RealRandom {
#ifdef _WIN32
        HCRYPTPROV handle_{ 0 };
#else
        int handle_{ -1 };
#endif

    public:
        RealRandom(const RealRandom&) noexcept = delete;
        RealRandom(RealRandom&&) noexcept = delete;
        RealRandom& operator=(const RealRandom&) noexcept = delete;
        RealRandom& operator=(RealRandom&&) noexcept = delete;

        ~RealRandom() {
#ifdef _WIN32
            if (handle_) {
                CryptReleaseContext(handle_, 0);
            }
#else
            if (handle_ >= 0) {
                close(handle_);
            }
#endif
        }

        static RealRandom& instance() {
            static RealRandom inst;
            return inst;
        }

        void random_bytes(void* buffer, size_t size) const {
            if (size == 0) {
                return;
            }
#ifdef _WIN32
            CryptGenRandom(handle_, static_cast<DWORD>(size), static_cast<BYTE*>(buffer));
#else
            auto p = static_cast<std::byte*>(buffer);
            do {
                size_t bytes_read = read(handle_, p, size);
                p += bytes_read;
                size -= bytes_read;
            } while (size > 0);
#endif
        }

    private:
        RealRandom() {
#ifdef _WIN32
            CryptAcquireContext(&handle_, nullptr, nullptr, PROV_RSA_FULL, 0);
            if (handle_ == 0) {
                throw std::runtime_error("crypt acquire context error");
            }
#else
            handle_ = open("/dev/urandom", O_RDONLY); //block: replace with "/dev/random"
            if (handle_ < 0) {
                throw std::runtime_error("open `/dev/urandom` error");
            }
#endif
        }
    };

    template <typename T>
    concept integral = std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool>;
}


void random_bytes(void* buffer, size_t size) {
    RealRandom::instance().random_bytes(buffer, size);
}

template <integral T>
T random_integer() {
    T res;
    random_bytes(&res, sizeof(T));
    return res;
}

template <integral T, template <typename> class Container = std::pmr::vector>
Container<T> random_integers(size_t size) {
    if (size == 0) {
        return Container<T>{};
    }

    Container<T> res(size);
    random_bytes(res.data(), size * sizeof(T));
    return res;
}

template <typename Str = std::pmr::string>
Str random_string(size_t size) {
    if (size == 0) {
        return Str{};
    }

    using char_type = typename Str::value_type;
    auto buffer = new char_type[size];
    random_bytes(buffer, size * sizeof(char_type));
    Str res(buffer, buffer + size);
    delete[] buffer;
    return res;
}
