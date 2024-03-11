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

#include "config.h"
#include "station.h"
#include "db.h"

// trim from start (in place)
//inline void ltrim(std::string &s) {
//    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
//        return !std::isspace(ch);
//    }));
//}

// trim from end (in place)
//inline void rtrim(std::string &s) {
//    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
//        return !std::isspace(ch);
//    }).base(), s.end());
//}

loglevel_e wloglevel = logDEBUG;

//const std::vector<std::string> config_list = {"LOG", "HOST", "DB", "STATIONS"};

class weather_gov
{

public:
    weather_gov() {
        std::cout << "weather_gov class constructor\n";
    }

};



int
main(int argc, char** argv) {
    wlog(logINFO) << "Welcome to Weather_gov";
    wlog(logWARNING) << "Weather_gov is under construction";
    wlog(logERROR) << "Weather_gov may produce errors";

    YAML::Node _config = YAML::LoadFile("./weather_gov.yml");
    assert(_config.Type() == YAML::NodeType::Map);
    wlog(logINFO) << "config: " << _config << "\n\n";

    Config config = Config(_config);

    std::string base_url;
    std::string stations_url;
    std::shared_ptr<std::map<std::string, std::string>> api_urls;

    try {
        api_urls = config.get_api_urls();
        base_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "base_url: "  << base_url;
        stations_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "stations_url: "  << stations_url;

    } catch (...) {
        wlog(logERROR) << "Exception getting API urls";
        return -1;
    }


    std::vector<Station> station_list;
    std::shared_ptr<std::map<std::string, std::string>> station_map = config.get_station_map();
    for (auto const& station : (*station_map))
    {
        //std::cout << station.first  // string (key)
        //      << ':' 
        //      << station.second // string's value 
        //      << std::endl;
        wlog(logINFO) << "Station name: " << station.first << " ID: " << station.second;
        station_list.push_back(Station(station.second, stations_url));
    }   

    //std::map<std::string, std::string> db_config;
    std::shared_ptr<std::map<std::string, std::string>> db_config = config.get_db_config();
    //for (auto const& db_item : (*db_config))
    //{
    //    std::cout << db_item.first  // string (key)
    //          << ':' 
    //          << db_item.second // string's value 
    //          << std::endl;
    //} 

    Db db = Db(db_config);

}
