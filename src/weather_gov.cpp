#include <iostream>

#include "yaml-cpp/yaml.h"

class weather_gov
{

public:
    weather_gov() {
        std::cout << "weather_gov class constructor\n";
    }

};



int
main(int argc, char** argv) {
    std::cout << "weather_gov main\n";

    YAML::Node config = YAML::LoadFile("./weather_gov.yml");
}
