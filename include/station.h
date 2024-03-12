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

private:
    std::string m_stationIdentifier;
    std::string m_stations_url;
    bool m_station_json_valid;
    std::string m_station_url;
    std::array<std::string, 1> m_station_json_data;

    std::string m_name;
    double m_longitude;
    double m_latitude;
    double m_elevation_meters;
    double m_elevation_feet;

    void get_station_json_data();
    void get_station_coordinates();

};

#endif
