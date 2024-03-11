

#include <iostream>
#include <sstream>
#include <memory>
#include "logger.h"


class Db
{
public:
    Db(std::shared_ptr<std::map<std::string, std::string>> db_config);

private:
    std::shared_ptr<std::map<std::string, std::string>> m_db_config;

    std::string m_host;
    std::string m_database;
    std::string m_user;
    std::string m_pass;
    std::string m_station_table;
    std::string m_observation_table; 

    bool ensure_database();
};

