
#include "config.h"


//* Config constructor, set the config
Config::Config(const YAML::Node config): m_config(config)
{
}


//* Get the host config for accessing weather.gov
bool
Config::get_host_map(std::map<std::string, std::string>& station_map) {
    for(const std::string& key : config_list) {

        if (m_config[key] && key == "HOST") {
            for(YAML::const_iterator it=m_config[key].begin();it!=m_config[key].end();++it) {
                station_map[it->first.as<std::string>()] = it->second.as<std::string>();
            }
        }
    }
    return true;
}


//* Get the station map (name and stationIdentifier)
bool
Config::get_station_map(std::map<std::string, std::string>& station_map) {
    for(const std::string& key : config_list) {

        if (m_config[key] && key == "STATIONS") {
            for(YAML::const_iterator it=m_config[key].begin();it!=m_config[key].end();++it) {
                station_map[it->first.as<std::string>()] = it->second.as<std::string>();
            }
        }
    }
    return true;
}


//* Get the mariadb configuration 
bool
Config::get_db_config(std::map<std::string, std::string>& db_config) {
    for(const std::string& key : config_list) {

        if (m_config[key] && key == "DB") {
            for(YAML::const_iterator it=m_config[key].begin();it!=m_config[key].end();++it) {
                db_config[it->first.as<std::string>()] = it->second.as<std::string>();
            }
        }
    }
    return true;

}

//* Get the parameters configuration
bool
Config::get_params_config(std::map<std::string, std::string>& params_config) {
    for(const std::string& key : config_list) {

        if (m_config[key] && key == "PARAMETERS") {
            for(YAML::const_iterator it=m_config[key].begin();it!=m_config[key].end();++it) {
                params_config[it->first.as<std::string>()] = it->second.as<std::string>();
            }
        }
    }
    return true;

}
