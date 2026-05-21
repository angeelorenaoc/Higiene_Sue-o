#pragma once
#ifndef PAYLOAD_HPP
#define PAYLOAD_HPP

#include <string>

namespace mqtt {
    struct payload {
        std::string message;

        std::string marshal(){
            return message;
        }
        static payload unmarshal(std::string payload){
            return {payload};
        }
    };
}


#endif // PAYLOAD_HPP
