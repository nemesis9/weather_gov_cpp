
#include "config.h"



extern loglevel_e wloglevel;


Config::Config(YAML::Node config): m_config(config)
{
}




std::shared_ptr<std::map<std::string, std::string>>
Config::get_api_urls()
{
    std::shared_ptr<std::map<std::string, std::string>> api_urls(new std::map<std::string, std::string>);
    std::string base_url;
    std::string stations_url;
    try {
        for(YAML::const_iterator it=m_config["HOST"].begin();it!=m_config["HOST"].end();++it) {
            std::cout << "HOST Key: " << it->first.as<std::string>() << " HOST Value: " << it->second.as<std::string>() << "\n" << std::flush;
            std::string key = it->first.as<std::string>();
            std::string value = it->second.as<std::string>();
            if (key  == "BASE_URL") {
                base_url = value;
                wlog(logINFO) << "base_url: "  << base_url << "\n";
            } else  if (key == "STATIONS_URL") {
                  stations_url = value;
                  wlog(logINFO) << "stations_url: "  << stations_url << "\n";
            } else {
                wlog(logERROR) << "Could not find API urls in config" << "\n";
                throw std::runtime_error("Could not find API urls in config");
            }
        }
    } catch (json::exception e) {
          wlog(logERROR) << "Exception parsing HOST apis";
          throw;
    }
    (*api_urls)["BASE_URL"] = base_url;
    (*api_urls)["STATIONS_URL"] = stations_url;
    return api_urls;

}


std::shared_ptr<std::map<std::string, std::string>>
Config::get_station_map() {
    std::shared_ptr<std::map<std::string, std::string>> station_map(new std::map<std::string, std::string>);
    for(const std::string& key : config_list) {
        std::cout << "key = " << key << std::endl;

        if (m_config[key] && key == "STATIONS") {
            wlog(logINFO) << "Processing stations\n";
            for(YAML::const_iterator it=m_config[key].begin();it!=m_config[key].end();++it) {
                wlog(logINFO) << "Key: " << it->first.as<std::string>() << " Value: " << it->second.as<std::string>();
                (*station_map)[it->first.as<std::string>()] = it->second.as<std::string>();
            }
        }
    }
    return station_map;
}
