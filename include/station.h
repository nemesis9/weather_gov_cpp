#ifndef _STATION_H
#define _STATION_H


#include <iostream>
#include <sstream>

#include "yaml-cpp/yaml.h"
#include <cassert>
#include <cpr/cpr.h>
#include <json.hpp>
#include <string>
#include <array>


using json = nlohmann::json;



class Station
{
public:
    Station(const std::string stationIdentifier, std::string stations_url);
    //Station(const Station&);
    //Station& operator=(const Station&) = delete;
    std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> get_record();
    //std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> get_latest_observation();
    bool get_latest_observation(std::shared_ptr<std::map<std::string, std::variant<std::string, float>>>&);

private:
    std::string m_stationIdentifier;
    std::string m_stations_url;
    bool m_station_json_valid;
    std::string m_station_url;
    std::array<std::string, 1> m_station_json_data;
    std::string m_observation_url;

    std::string m_name;
    double m_longitude;
    double m_latitude;
    double m_elevation_meters;
    double m_elevation_feet;

    void get_station_json_data();
    //void get_station_coordinates();

    void get_station_id(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_timestamp(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_temperature(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_dewpoint(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_description(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_winddir(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_windspeed(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_windgust(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_barometric_pressure(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);
    void get_rel_humidity(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map);

};

#endif
