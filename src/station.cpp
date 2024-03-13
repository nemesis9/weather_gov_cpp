#include "logger.h"
#include "station.h"
#include <cpr/cpr.h>
#include <json.hpp>

#include <typeinfo>

extern loglevel_e wloglevel;

Station::Station(const std::string stationIdentifier, const std::string stations_url) :
                        m_stationIdentifier(stationIdentifier),
                        m_stations_url(stations_url),
                        m_station_json_valid(false) {

    wlog(logINFO) << "Station constructor: identifier: " << stationIdentifier << "\n";
    m_station_url = m_stations_url + "/stations/" + m_stationIdentifier;
    wlog(logINFO) << "Stations url: " << m_stations_url;
    wlog(logINFO) << "Station created with url: " << m_station_url;
    m_observation_url = m_station_url + "/observations/latest";
    get_station_json_data();
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
        wlog(logINFO) << "JSON Doc: " << Doc;
        m_name = std::string(Doc[0]["properties"]["name"]);
        wlog(logINFO) << "Name: <" << m_name << ">";
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

    } catch (const json::exception &e) {
        wlog(logERROR) << "Exception parsing station json: " << e.what() << "\n";
        m_station_json_valid = false;
        m_station_json_data = {};
    }

}



std::shared_ptr<std::map<std::string, std::variant<std::string, float>>>
Station::get_record() {
    std::shared_ptr<std::map<std::string, std::variant<std::string, float>>>
        station_record(new std::map<std::string, std::variant<std::string, float>>);

    (*station_record)["call_id"] = m_stationIdentifier;
    (*station_record)["name"] = m_name;
    (*station_record)["latitude_deg"] = float(m_latitude);
    (*station_record)["longitude_deg"] = float(m_longitude);
    (*station_record)["elevation_m"] = float(m_elevation_meters);
    (*station_record)["url"] = m_station_url;

    return station_record;
}



//std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> 
bool
Station::get_latest_observation(std::shared_ptr<std::map<std::string, std::variant<std::string, float>>>& obs_map) {
    
    //std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map( 
    //    new std::map<std::string, std::variant<std::string, float>>);
    cpr::Response r = cpr::Get(cpr::Url{m_observation_url});
    wlog(logINFO) << "Status code: " << r.status_code << "\n";                  // 200
    wlog(logINFO) << "header: " << r.header["content-type"] << "\n";       // application/json; charset=utf-8
    wlog(logINFO) << "text: " << r.text << "\n";                         // JSON text string
                                                                         //
    if (r.status_code != 200) {
        wlog(logERROR) << "ERROR getting station observation. Return code: " << r.status_code;
        return false;
    }

    std::string resp = r.text;
    try {
        json Doc{json::parse(resp)};
        wlog(logINFO) << "JSON Doc: " << Doc;

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


void
Station::get_station_id(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    (*obs_map)["station_id"] = m_stationIdentifier;
    wlog(logINFO) << "Inserted station_id into obs_map: " << m_stationIdentifier;
 
    return;
}


void
Station::get_timestamp(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        std::string timestamp = std::string(Doc[0]["properties"]["timestamp"]);
        wlog(logINFO) << "Doc timestamp : " << timestamp;
        (*obs_map)["timestamp_UTC"] = timestamp;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json timestamp: " << e.what();
        (*obs_map)["timestamp_UTC"] = "None";
        wlog(logWARNING) << "Doc timestamp setting to None";
    }

    return;
}


void
Station::get_temperature(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float temperature = float(Doc[0]["properties"]["temperature"]["value"]);
        (*obs_map)["temperature_C"] = float(temperature);
        (*obs_map)["temperature_F"] = temperature * float((9.0/5.0)) + float(32.0);
        wlog(logINFO) << "Doc temperature_C : " << temperature;
        wlog(logINFO) << "Doc temperature_F : " << temperature * float((9.0/5.0)) + float(32.0);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json temperature: " << e.what();
        (*obs_map)["temperature_C"] = float(-999.99);
        (*obs_map)["temperature_F"] = float(-999.99);
        wlog(logWARNING) << "Doc temperature setting null to -999.99";
    }

    return;
}


void
Station::get_dewpoint(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float dewpoint = float(Doc[0]["properties"]["dewpoint"]["value"]);
        (*obs_map)["dewpoint_C"] = float(dewpoint);
        (*obs_map)["dewpoint_F"] = dewpoint * float((9.0/5.0)) + float(32.0);
        wlog(logINFO) << "Doc dewpoint_C : " << dewpoint;
        wlog(logINFO) << "Doc dewpoint_F : " << dewpoint * float((9.0/5.0)) + float(32.0);
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json dewpoint: " << e.what();
        (*obs_map)["dewpoint_C"] = float(-999.99);
        (*obs_map)["dewpoint_F"] = float(-999.99);
        wlog(logWARNING) << "Doc dewpoint setting null to -999.99";
    }

    return;
}


void
Station::get_description(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        std::string description = std::string(Doc[0]["properties"]["textDescription"]);
        wlog(logINFO) << "Doc description : " << description;
        (*obs_map)["description"] = description;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json description: " << e.what();
        (*obs_map)["description"] = "None";
        wlog(logWARNING) << "Doc description set to None";
    }

    return;
}


void
Station::get_winddir(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float windir = float(Doc[0]["properties"]["windDirection"]["value"]);
        (*obs_map)["wind_dir"] = float(windir);
        wlog(logINFO) << "Doc windir : " << windir;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind direction: " << e.what();
        (*obs_map)["wind_dir"] = float(-999.99);
        wlog(logWARNING) << "Doc winddir setting null to -999.99";
    }

    return;
}


void
Station::get_windspeed(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float windspeed = float(Doc[0]["properties"]["windSpeed"]["value"]);
        (*obs_map)["wind_spd_km_h"] = float(windspeed);
        (*obs_map)["wind_spd_mi_h"] = float(windspeed) * float(0.6213712);
        wlog(logINFO) << "Doc windspeed km/h : " << windspeed;
        wlog(logINFO) << "Doc windspeed mi/h : " << windspeed * float(0.6213712);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind speed: " << e.what();
        (*obs_map)["wind_spd_km_h"] = float(-999.99);
        (*obs_map)["wind_spd_mi_h"] = float(-999.99);
        wlog(logWARNING) << "Doc windspeed setting null to -999.99";
    }

    return;
}


void
Station::get_windgust(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float windgust = float(Doc[0]["properties"]["windGust"]["value"]);
        (*obs_map)["wind_gust_km_h"] = float(windgust);
        (*obs_map)["wind_gust_mi_h"] = float(windgust) * float(0.6213712);
        wlog(logINFO) << "Doc windgust km/h : " << windgust;
        wlog(logINFO) << "Doc windgust mi/h : " << windgust * float(0.6213712);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json wind gust: " << e.what();
        (*obs_map)["wind_gust_km_h"] = float(-999.99);
        (*obs_map)["wind_gust_mi_h"] = float(-999.99);
        wlog(logWARNING) << "Doc windgust setting null to -999.99";
    }

    return;
}


void
Station::get_barometric_pressure(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float baro_pres_pa = float(Doc[0]["properties"]["barometricPressure"]["value"]);
        (*obs_map)["baro_pres_pa"] = float(baro_pres_pa);
        (*obs_map)["baro_pres_inHg"] = float(baro_pres_pa) * float(0.00029529983071445);
        wlog(logINFO) << "Doc baro_pres_pa km/h : " << baro_pres_pa;
        wlog(logINFO) << "Doc baro_pres_inHg : " << baro_pres_pa * float(0.00029529983071445);

    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json barometricPressure: " << e.what();
        (*obs_map)["baro_pres_pa"] = float(-999.99);
        (*obs_map)["baro_pres_inHg"] = float(-999.99);
        wlog(logWARNING) << "Doc barometricPressure setting null to -999.99";
    }

    return;
}


void 
Station::get_rel_humidity(json Doc,  std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs_map) {

    try {
        float rel_humidity = float(Doc[0]["properties"]["relativeHumidity"]["value"]);
        (*obs_map)["rel_humidity"] = float(rel_humidity);
        wlog(logINFO) << "Doc rel_humidity : " << rel_humidity;
    } catch (const json::exception& e) {
        wlog(logERROR) << "Exception parsing obs json rel_humidity: " << e.what();
        (*obs_map)["rel_humidity"] = float(-999.99);
        wlog(logWARNING) << "Doc rel_humidity setting null to -999.99";
    }

    return;

}


//void
//Station::get_station_coordinates() {
//    if (m_station_json_valid) {
//        //json Doc(json::parse(m_station_json_data));
//        //wlog(logINFO) << "DOC: " << Doc << "\n";
//        //std::cout <<  m_station_json_data["geometry"]["coordinates"] << "\n";
//        //wlog(logINFO) << "Coordinates: " << coordinates << "\n";
//
//
//    }
//}
