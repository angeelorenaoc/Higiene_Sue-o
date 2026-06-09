#include "crypto.hpp"

#include <string>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace util::crypto {
    std::string sha256(std::string_view input){
        unsigned char hash[SHA256_DIGEST_LENGTH];

        SHA256(
            reinterpret_cast<const unsigned char*>(input.data()),
            input.size(),
            hash
        );

        std::stringstream ss;

        for (unsigned char byte : hash)
            ss << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<int>(byte);

        return ss.str();
    }
}
