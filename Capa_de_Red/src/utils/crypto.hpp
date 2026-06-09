#pragma once
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <string>
#include <string_view>


namespace util::crypto {
    std::string sha256(std::string_view input);
}

#endif
