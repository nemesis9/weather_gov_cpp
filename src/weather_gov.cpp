
#include "yaml-cpp/yaml.h"
#include <cassert>
#include <string>
#include <cpr/cpr.h>
#include "logger.h"
#include "config.h"
#include "station.h"
#include "db.h"


//*
//* Set the global log level
//*
loglevel_e wloglevel = logINFO;



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

    //* Instantiate our config object
    Config config = Config(_config);

    //* Allocate objects to receive basic info
    std::string base_url;
    std::string stations_url;
    std::map<std::string, std::string> api_urls;
    std::map<std::string, std::string> params;

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


    //* Get the params
    int obs_interval = 30;
    try {
        config.get_params_config(params);
        obs_interval = std::stoi(params["OBS_INTERVAL_SECS"]);

    } catch (...) {
        wlog(logERROR) << "Exception getting params";
        wlog(logWARNING) << "Setting obs interval to default 30 seconds";
    }

    //* Create a station list
    std::vector<Station> station_list;
    std::map<std::string, std::string> station_map;
    config.get_station_map(station_map);
    for (auto const& station : station_map)
    {
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
        db.put_station_record(station_record);
    }

    //* loop over stations and store the latest weather observation
    bool loop = true;
    std::map<std::string, std::variant<std::string, float>> obs; 
    while (true) {

        for (auto s: station_list) {
            wlog(logINFO) << "Getting latest observation for station " << s.get_station_identifier();
            s.get_latest_observation(obs);
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
        std::this_thread::sleep_for(std::chrono::seconds(obs_interval));
        
    }
}
