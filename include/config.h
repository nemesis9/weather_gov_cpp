
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include "yaml-cpp/yaml.h"
#include "logger.h"

#include <json.hpp>
using json = nlohmann::json;


const std::vector<std::string> config_list = {"LOG", "HOST", "DB", "STATIONS"};

class Config
{
public:
    Config(YAML::Node config);

    //std::shared_ptr<std::map<std::string, std::string>>
    bool get_api_urls(std::map<std::string, std::string>*);


    std::shared_ptr<std::map<std::string, std::string>>
        get_station_map();

    std::shared_ptr<std::map<std::string, std::string>>
        get_db_config();

private:
    YAML::Node m_config;



};
