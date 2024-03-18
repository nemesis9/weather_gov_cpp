
#include "yaml-cpp/yaml.h"
#include <cassert>
#include <cpr/cpr.h>
#include "logger.h"
#include "config.h"
#include "station.h"
#include "db.h"


//*
//* Set the global log level
//*
loglevel_e wloglevel = logDEBUG;

//*
//* main
//*
int
main(int argc, char** argv) {
    /* A little log test */
    wlog(logINFO) << "Welcome to Weather_gov";
    wlog(logWARNING) << "Weather_gov is under construction";
    wlog(logERROR) << "Weather_gov may produce errors";

    //* See if we can load the config. Bomb if we dont get it.                
    //* There is cmake copy target that copies the yml file
    //*     to the build directory so it is current directory
    YAML::Node _config = YAML::LoadFile("./weather_gov.yml");
    assert(_config.Type() == YAML::NodeType::Map);
    wlog(logINFO) << "config: " << _config << "\n\n";

    //* Instantiate our config object
    Config config = Config(_config);

    //* Allocate objects to receive basic info
    std::string base_url;
    std::string stations_url;
    std::map<std::string, std::string> api_urls;

    //* Try to get the basic data 
    try {
        config.get_api_urls(api_urls);
        base_url = api_urls["BASE_URL"];
        wlog(logINFO) << "base_url: "  << base_url;
        stations_url = api_urls["BASE_URL"];
        wlog(logINFO) << "stations_url: "  << stations_url;

    } catch (...) {
        wlog(logERROR) << "Exception getting API urls";
        return -1;
    }

    //* Create a station list
    std::vector<Station> station_list;
    std::map<std::string, std::string> station_map;
    config.get_station_map(station_map);
    for (auto const& station : station_map)
    {
        wlog(logINFO) << "Creating station:  name: " << station.first << " ID: " << station.second;
        station_list.push_back(Station(station.second, stations_url));
    }

    //* Create a db object
    std::map<std::string, std::string> db_config;
    config.get_db_config(db_config);

    Db db = Db(db_config);

    //* For all the stations in the config, create a db record if we don't
    //      already have one
    for (auto s: station_list) {
        std::map<std::string, std::variant<std::string, float>> station_record;
        s.get_station_record(station_record);
        std::string call_id = std::get<std::string>(station_record["call_id"]);
        wlog(logDEBUG) << "call_id: " << call_id;
        std::string name = std::get<std::string>(station_record["name"]);
        wlog(logDEBUG) << "name: " << name;
        float latitude = std::get<float>(station_record["latitude_deg"]);
        wlog(logDEBUG) << "latitude: " << latitude;
        float longitude = std::get<float>(station_record["longitude_deg"]);
        wlog(logDEBUG) << "longitude: " << longitude;
        float elevation = std::get<float>(station_record["elevation_m"]);
        wlog(logDEBUG) << "elevation: " << elevation;
        std::string url = std::get<std::string>(station_record["url"]);
        wlog(logDEBUG) << "url: " << url;

        db.put_station_record(station_record);
    }

    //* loop over stations and store the latest weather observation
    bool loop = true;
    std::map<std::string, std::variant<std::string, float>> obs; 
    while (true) {

        for (auto s: station_list) {
            s.get_latest_observation(obs);
            for (auto item: obs) {
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
                 } else {
                     wlog(logERROR) << "weather_gov: ERROR storing station observation" 
                                " for station " << std::get<std::string>(obs["station_id"]) << "\n";
                 }
            }
        }

        if (false == loop) break;
        //* Loop every 5 minutes (should be a config item)
        std::this_thread::sleep_for(std::chrono::seconds(300));
        
    }
}
