#include <iostream>
#include <sstream>

#include "yaml-cpp/yaml.h"
#include <cassert>
#include "logger.h"
#include <cpr/cpr.h>
#include <json.hpp>
using json = nlohmann::json;

#include <algorithm>
#include <cctype>
#include <locale>
#include <vector>
#include <stdexcept>
#include <memory>

#include "station.h"

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

loglevel_e wloglevel = logDEBUG;

const std::vector<std::string> config_list = {"LOG", "HOST", "DB", "STATIONS"};

class weather_gov
{

public:
    weather_gov() {
        std::cout << "weather_gov class constructor\n";
    }

};


std::shared_ptr<std::map<std::string, std::string>>
get_api_urls(YAML::Node config) {
    std::shared_ptr<std::map<std::string, std::string>> api_urls(new std::map<std::string, std::string>);
    std::string base_url;
    std::string stations_url;
    try {
        for(YAML::const_iterator it=config["HOST"].begin();it!=config["HOST"].end();++it) {
            std::cout << "HOST Key: " << it->first.as<std::string>() << " HOST Value: " << it->second.as<std::string>() << "\n" << std::flush;
            std::string key = it->first.as<std::string>();
            std::string value = it->second.as<std::string>();
            if (key  == "BASE_URL") {
                base_url = value;
                wlog(logINFO) << "base_url: "  << base_url << "\n";
            } else  if (key == "STATIONS_URL") {
                  stations_url = value;
                  wlog(logINFO) << "stations_url: "  << stations_url << "\n";
            } else {
                wlog(logERROR) << "Could not find API urls in config" << "\n";
                throw std::runtime_error("Could not find API urls in config");
            }
        }
    } catch (json::exception e) {
          wlog(logERROR) << "Exception parsing HOST apis";
          throw;
    }
    (*api_urls)["BASE_URL"] = base_url;
    (*api_urls)["STATIONS_URL"] = stations_url;
    return api_urls;
}


int
main(int argc, char** argv) {
    std::cout << "weather_gov main\n";
    wlog(logINFO) << "Welcome to Weather_gov";
    wlog(logWARNING) << "Weather_gov is under construction";
    wlog(logERROR) << "Weather_gov may produce errors";

    YAML::Node config = YAML::LoadFile("./weather_gov.yml");
    assert(config.Type() == YAML::NodeType::Map);

    std::cout << "config: " << config << "\n\n";
    std::string base_url;
    std::string stations_url;
    std::shared_ptr<std::map<std::string, std::string>> api_urls;

    try {
        api_urls = get_api_urls(config);
        base_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "base_url: "  << base_url;
        stations_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "stations_url: "  << stations_url;

    } catch (...) {
        wlog(logERROR) << "Exception getting API urls";
        return -1;
    }


    std::vector<Station> station_list;
    for(const std::string& key : config_list) {
        std::cout << "key = " << key << std::endl;

        if (config[key] && key == "STATIONS") {
            wlog(logINFO) << "Processing stations\n";
            for(YAML::const_iterator it=config[key].begin();it!=config[key].end();++it) {
                std::cout << "Key: " << it->first.as<std::string>() << " Value: " << it->second.as<std::string>() << "\n" << std::flush;
                station_list.push_back(Station(it->second.as<std::string>(), stations_url));
            }
        }
    }
}
