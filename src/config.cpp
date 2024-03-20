
#include "config.h"


//* Config constructor, set the config
Config::Config(YAML::Node config): m_config(config)
{
}

//* Get the api_urls for accessing weather.gov
bool
Config::get_api_urls(std::map<std::string, std::string>& api_urls)
{
    std::string base_url;
    std::string stations_url;
    try {
        for(YAML::const_iterator it=m_config["HOST"].begin();it!=m_config["HOST"].end();++it) {
            std::string key = it->first.as<std::string>();
            std::string value = it->second.as<std::string>();
            if (key  == "BASE_URL") {
                base_url = value;
            } else  if (key == "STATIONS_URL") {
                  stations_url = value;
            } else {
                wlog(logERROR) << "Could not find API urls in config" << "\n";
                throw std::runtime_error("Could not find API urls in config");
            }
        }
    } catch (const json::exception &e) {
          wlog(logERROR) << "Exception parsing HOST apis";
          throw;
    }
    api_urls["BASE_URL"] = base_url;
    api_urls["STATIONS_URL"] = stations_url;
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
