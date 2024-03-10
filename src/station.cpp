#include "logger.h"
#include "station.h"
#include <cpr/cpr.h>
#include <json.hpp>

#include <typeinfo>

extern loglevel_e wloglevel;

Station::Station(const std::string stationIdentifier, const std::string stations_url) : m_station_json_valid(false),
                        m_stationIdentifier(stationIdentifier),
                        m_stations_url(stations_url)
{
    wlog(logINFO) << "Station constructor: identifier: " << stationIdentifier << "\n";
    m_station_url = m_stations_url + "/stations/" + m_stationIdentifier;
    wlog(logINFO) << "Stations url: " << m_stations_url;
    wlog(logINFO) << "Station created with url: " << m_station_url;
    get_station_json_data();
    //get_station_coordinates();
}

void
Station::get_station_json_data() {

    cpr::Response r = cpr::Get(cpr::Url{m_station_url});
    wlog(logINFO) << "Status code: " << r.status_code << "\n";                  // 200
    wlog(logINFO) << "header: " << r.header["content-type"] << "\n";       // application/json; charset=utf-8
    wlog(logINFO) << "text: " << r.text << "\n";                         // JSON text string
    if (r.status_code != 200) {
        wlog(logERROR) << "ERROR getting station data. Return code: " << r.status_code;
        return;
    }

    std::string str = r.text;
    size_t strt = str.find("context");
    size_t end = str.find("],");
    wlog(logDEBUG) << "start: " << strt << " end: " << end << "\n";
    str.erase(strt-1, end+1);
    wlog(logINFO) << "JSON String after: " << str << "\n";

    try {
        json Doc{json::parse(str)};
        std::cout << Doc << "\n";
        std::string name = std::string(Doc[0]["properties"]["name"]);
        wlog(logINFO) << "Name: <" << name << ">";
        wlog(logINFO) << "Coords " << Doc[0]["geometry"]["coordinates"];
        std::array<double, 2> coords = std::array<double, 2>(Doc[0]["geometry"]["coordinates"]);

        m_longitude = coords[0];
        m_latitude = coords[1];
        wlog(logINFO) << "LONGITUDE: <" << m_longitude << ">";
        wlog(logINFO) << "LATITUDE: <" << m_latitude << ">";

        m_elevation_meters = Doc[0]["properties"]["elevation"]["value"];
        m_elevation_feet = m_elevation_meters * 3.28084;
        wlog(logINFO) << "Elevation (m): " << m_elevation_meters << "\n";
        wlog(logINFO) << "Elevation (ft): " << m_elevation_feet << "\n";

    } catch (json::exception e) {
        wlog(logERROR) << "Exception parsing json: " << e.what() << "\n";
        m_station_json_valid = false;
        m_station_json_data = {};
    }

    //std::cout << Doc << "\n";
    //std::cout << "GEOMETRY: " << Doc[0]["geometry"] << "\n";
    //std::cout << "COORDINATES: " << Doc[0]["geometry"]["coordinates"] << "\n";

    //std::cout << "NAME: " << Doc[0]["properties"]["name"] << "\n";
    //std::cout << "IDENTIFIER: " << Doc[0]["properties"]["stationIdentifier"] << "\n";
}


void
Station::get_station_coordinates() {
    if (m_station_json_valid) {
        //json Doc(json::parse(m_station_json_data));
        //wlog(logINFO) << "DOC: " << Doc << "\n";
        //std::cout <<  m_station_json_data["geometry"]["coordinates"] << "\n";
        //wlog(logINFO) << "Coordinates: " << coordinates << "\n";


    }

}
