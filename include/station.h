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
    const std::string& get_station_identifier();
    bool get_station_record(std::map<std::string, std::variant<std::string, float>>&);
    bool get_latest_observation(std::map<std::string, std::variant<std::string, float>>&);

private:
    std::string m_stationIdentifier;
    std::string m_stations_url;
    bool m_station_json_valid;
    std::string m_station_url;
    std::string m_observation_url;

    std::string m_name;
    double m_longitude;
    double m_latitude;
    double m_elevation_meters;
    double m_elevation_feet;

    void get_station_json_data();

    void get_station_id(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_timestamp(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_temperature(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_dewpoint(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_description(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_winddir(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_windspeed(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_windgust(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_barometric_pressure(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);
    void get_rel_humidity(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map);

};

#endif
