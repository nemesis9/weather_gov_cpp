
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include "yaml-cpp/yaml.h"
#include "logger.h"

#include <json.hpp>
using json = nlohmann::json;


const std::vector<std::string> config_list = {"LOG", "HOST", "DB", "STATIONS", "PARAMETERS"};

class Config
{
public:
    Config(const YAML::Node config);

    bool get_api_urls(std::map<std::string, std::string>&);

    bool get_station_map(std::map<std::string, std::string>&);

    bool get_db_config(std::map<std::string, std::string>&);

    bool get_params_config(std::map<std::string, std::string>&);
private:
    YAML::Node m_config;



};
