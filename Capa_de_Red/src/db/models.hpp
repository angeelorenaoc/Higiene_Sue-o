#pragma once
#ifndef DB_MODELS_HPP
#define DB_MODELS_HPP

#include <filesystem>

namespace db {
    struct migration {
        int version;
        std::filesystem::path path;
    };
}

#endif
