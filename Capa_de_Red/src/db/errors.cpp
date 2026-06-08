#include "errors.hpp"

namespace db {
    std::string errorToString(Error e){
        switch (e) {
            case Error::FAILED_OPEN_DB: return "failed to open db";
            case Error::NOT_PRESENT: return "value not present";
            case Error::FAILED: return "operation failed";
            default: return "undefined message";
        }
    }
}
