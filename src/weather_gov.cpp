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
#include <chrono>
#include <thread>

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
    //std::shared_ptr<std::map<std::string, std::string>> api_urls;
    //std::shared_ptr<std::map<std::string, std::string>> api_urls(new std::map<std::string, std::string>);
    std::map<std::string, std::string>* api_urls(new (std::nothrow) std::map<std::string, std::string>);
    if (!api_urls) {
        wlog(logERROR) << "Could not allocate memory to get api_urls";
        return -1;
    }

    try {
        bool ret = config.get_api_urls(api_urls);
        base_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "base_url: "  << base_url;
        stations_url = (*api_urls)["BASE_URL"];
        wlog(logINFO) << "stations_url: "  << stations_url;

    } catch (...) {
        wlog(logERROR) << "Exception getting API urls";
        return -1;
    }
    delete api_urls;

    std::vector<Station> station_list;
    std::shared_ptr<std::map<std::string, std::string>> station_map = config.get_station_map();
    for (auto const& station : (*station_map))
    {
        wlog(logINFO) << "Creating station:  name: " << station.first << " ID: " << station.second;
        station_list.push_back(Station(station.second, stations_url));
    }

    std::shared_ptr<std::map<std::string, std::string>> db_config = config.get_db_config();

    Db db = Db(db_config);

    for (auto s: station_list) {
        std::map<std::string, std::variant<std::string, float>>* station_record = 
            new std::map<std::string, std::variant<std::string, float>>;
        s.get_station_record(station_record);
        std::string call_id = std::get<std::string>((*station_record)["call_id"]);
        wlog(logDEBUG) << "call_id: " << call_id;
        std::string name = std::get<std::string>((*station_record)["name"]);
        wlog(logDEBUG) << "name: " << name;
        float latitude = std::get<float>((*station_record)["latitude_deg"]);
        wlog(logDEBUG) << "latitude: " << latitude;
        float longitude = std::get<float>((*station_record)["longitude_deg"]);
        wlog(logDEBUG) << "longitude: " << longitude;
        float elevation = std::get<float>((*station_record)["elevation_m"]);
        wlog(logDEBUG) << "elevation: " << elevation;
        std::string url = std::get<std::string>((*station_record)["url"]);
        wlog(logDEBUG) << "url: " << url;

        db.put_station_record(station_record);
    }

    bool loop = false;
    while (true) {
        //std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs;

        std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs( 
            new std::map<std::string, std::variant<std::string, float>>);
        for (auto s: station_list) {
            s.get_latest_observation(obs);
            for (auto item: (*obs)) {
                wlog(logINFO) << "weather_gov: obs item: Key:" << item.first;
                if ((item.first == "station_id") || (item.first == "description") ||
                     (item.first == "timestamp_UTC")) {
                    wlog(logINFO) << "weather_gov: obs       Value: " << std::get<std::string>(item.second);
                } else {
                    wlog(logINFO) << "weather_gov: obs       Value: " << std::get<float>(item.second);
                }
            }
            std::tuple<bool, std::string> ret = db.put_observation(obs);
            if (std::get<0>(ret) == false) {
                std::string res = std::get<1>(ret);
                 if (res.find("Duplicate") != std::string::npos) {
                     wlog(logINFO) << "weather_gov igoring duplicate record";
                 }
            }
        }

        if (obs.unique()) {
            wlog(logINFO) << "obs ptr is unique, resetting";
            obs.reset();
        } else {
            wlog(logINFO) << "obs ptr is NOT unique";
        }
        if (false == loop) break;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
    }
}
