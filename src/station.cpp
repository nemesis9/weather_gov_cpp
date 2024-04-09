#include "logger.h"
#include "station.h"
#include <cpr/cpr.h>
#include <json.hpp>

#include <typeinfo>


//* PUBLIC
//* Station object needs to know the url and identifier
Station::Station(const std::string stationIdentifier, const std::string stations_url) :
                        m_stationIdentifier(stationIdentifier),
                        m_stations_url(stations_url),
                        m_station_json_valid(false) {

    m_station_url = m_stations_url + m_stationIdentifier;
    wlog(logINFO) << "Station created with url: " << m_station_url;
    m_observation_url = m_station_url + "/observations/latest";
    get_station_json_data();
}


//* PUBLIC
//* Convenience funtion to return the station identifier
const std::string&
Station::get_station_identifier() {
    return m_stationIdentifier;
}


//* Station interface to return station data
bool
Station::get_station_record(std::map<std::string, std::variant<std::string, float>>& station_record
) {

    station_record["call_id"] = m_stationIdentifier;
    station_record["name"] = m_name;
    station_record["latitude_deg"] = float(m_latitude);
    station_record["longitude_deg"] = float(m_longitude);
    station_record["elevation_m"] = float(m_elevation_meters);
    station_record["url"] = m_station_url;

    return true;
}


//* PUBLIC
//* Station interface to return latest station observation data
bool
Station::get_latest_observation(std::map<std::string, std::variant<std::string, float>>& obs_map) {

    cpr::Response r = cpr::Get(cpr::Url{m_observation_url});
    wlog(logDEBUG) << "Status code: " << r.status_code << "\n";          // 200

    if (r.status_code != 200) {
        wlog(logERROR) << "ERROR getting station observation. Return code: " << r.status_code;
        return false;
    }

    std::string resp = r.text;
    try {
        json Doc{json::parse(resp)};

        get_station_id(Doc, obs_map);
        get_timestamp(Doc, obs_map);
        get_temperature(Doc, obs_map);
        get_dewpoint(Doc, obs_map);
        get_description(Doc, obs_map);
        get_winddir(Doc, obs_map);
        get_windspeed(Doc, obs_map);
        get_windgust(Doc, obs_map);
        get_barometric_pressure(Doc, obs_map);
        get_rel_humidity(Doc, obs_map);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json: " << e.what();
        return false;
    }
    //return obs_map;
    return true;
}


//* PRIVATE
//* Station object get station data
void
Station::get_station_json_data() {

    cpr::Response r = cpr::Get(cpr::Url{m_station_url});
    wlog(logDEBUG) << "GET Status code: " << r.status_code << "\n";                  // 200
    if (r.status_code != 200) {
        wlog(logERROR) << "ERROR getting station data. Return code: " << r.status_code;
        return;
    }

    std::string str = r.text;
    size_t strt = str.find("context");
    size_t end = str.find("],");
    str.erase(strt-1, end+1);

    try {
        json Doc{json::parse(str)};
        wlog(logDEBUG) << "JSON Doc: " << Doc;
        m_name = std::string(Doc[0]["properties"]["name"]);
        std::array<double, 2> coords = std::array<double, 2>(Doc[0]["geometry"]["coordinates"]);

        m_longitude = coords[0];
        m_latitude = coords[1];

        m_elevation_meters = Doc[0]["properties"]["elevation"]["value"];
        m_elevation_feet = m_elevation_meters * 3.28084;

        m_station_json_valid = true;
    } catch (const json::exception &e) {
        wlog(logERROR) << "Exception parsing station json: " << e.what() << "\n";
        m_longitude = m_latitude = m_elevation_meters = m_elevation_feet = 0.0;
        m_station_json_valid = false;
    }

}



//* PRIVATE
//* Station get station id from observation data and put into map
void
Station::get_station_id(const json& Doc, std::map<std::string, std::variant<std::string, float>>& obs_map) {

    obs_map["station_id"] = m_stationIdentifier;

    return;
}


//* PRIVATE
//* Station get timestamp from observation data and put into map
void
Station::get_timestamp(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        std::string timestamp = std::string(Doc[0]["properties"]["timestamp"]);
        obs_map["timestamp_UTC"] = timestamp;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json timestamp: " << e.what();
        obs_map["timestamp_UTC"] = "None";
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : timestamp setting to None";
    }

    return;
}


//* PRIVATE
//* Station get temperature from observation data and put into map
void
Station::get_temperature(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float temperature = float(Doc[0]["properties"]["temperature"]["value"]);
        obs_map["temperature_C"] = float(temperature);
        obs_map["temperature_F"] = temperature * float((9.0/5.0)) + float(32.0);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json temperature: " << e.what();
        obs_map["temperature_C"] = float(-999.99);
        obs_map["temperature_F"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : temperature setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get dewpoint from observation data and put into map
void
Station::get_dewpoint(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float dewpoint = float(Doc[0]["properties"]["dewpoint"]["value"]);
        obs_map["dewpoint_C"] = float(dewpoint);
        obs_map["dewpoint_F"] = dewpoint * float((9.0/5.0)) + float(32.0);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json dewpoint: " << e.what();
        obs_map["dewpoint_C"] = float(-999.99);
        obs_map["dewpoint_F"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : dewpoint setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get description from observation data and put into map
void
Station::get_description(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        std::string description = std::string(Doc[0]["properties"]["textDescription"]);
        obs_map["description"] = description;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json description: " << e.what();
        obs_map["description"] = "None";
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : description set to None";
    }

    return;
}


//* PRIVATE
//* Station get wind direction from observation data and put into map
void
Station::get_winddir(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float windir = float(Doc[0]["properties"]["windDirection"]["value"]);
        obs_map["wind_dir"] = float(windir);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind direction: " << e.what();
        obs_map["wind_dir"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : winddir setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get wind speed from observation data and put into map
void
Station::get_windspeed(const json& Doc, std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float windspeed = float(Doc[0]["properties"]["windSpeed"]["value"]);
        obs_map["wind_spd_km_h"] = float(windspeed);
        obs_map["wind_spd_mi_h"] = float(windspeed) * float(0.6213712);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind speed: " << e.what();
        obs_map["wind_spd_km_h"] = float(-999.99);
        obs_map["wind_spd_mi_h"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : windspeed setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get wind gust from observation data and put into map
void
Station::get_windgust(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float windgust = float(Doc[0]["properties"]["windGust"]["value"]);
        obs_map["wind_gust_km_h"] = float(windgust);
        obs_map["wind_gust_mi_h"] = float(windgust) * float(0.6213712);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind gust: " << e.what();
        obs_map["wind_gust_km_h"] = float(-999.99);
        obs_map["wind_gust_mi_h"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : windgust setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get barometric pressure from observation data and put into map
void
Station::get_barometric_pressure(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float baro_pres_pa = float(Doc[0]["properties"]["barometricPressure"]["value"]);
        obs_map["baro_pres_pa"] = float(baro_pres_pa);
        obs_map["baro_pres_inHg"] = float(baro_pres_pa) * float(0.00029529983071445);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json barometricPressure: " << e.what();
        obs_map["baro_pres_pa"] = float(-999.99);
        obs_map["baro_pres_inHg"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : barometricPressure setting null to -999.99";
    }

    return;
}


//* PRIVATE
//* Station get relative humidity from observation data and put into map
void
Station::get_rel_humidity(const json& Doc,  std::map<std::string, std::variant<std::string, float>>& obs_map) {

    try {
        float rel_humidity = float(Doc[0]["properties"]["relativeHumidity"]["value"]);
        obs_map["rel_humidity"] = float(rel_humidity);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json rel_humidity: " << e.what();
        obs_map["rel_humidity"] = float(-999.99);
        wlog(logWARNING) << "Station " << m_stationIdentifier << " : rel_humidity setting null to -999.99";
    }

    return;

}


