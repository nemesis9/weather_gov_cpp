

#include <iostream>
#include <sstream>
#include <memory>
#include "logger.h"

#include <mariadb/conncpp.hpp>

class Db
{
public:
    Db(const std::map<std::string, std::string>& db_config);

    bool put_station_record (std::map<std::string, std::variant<std::string, float>>&);

    std::tuple<bool, std::string>
    put_observation (std::map<std::string, std::variant<std::string, float>>&);

private:
    std::map<std::string, std::string> m_db_config;

    std::string m_host;
    std::string m_port;
    std::string m_database;
    std::string m_user;
    std::string m_pass;
    std::string m_station_table;
    std::string m_observation_table;

    std::string make_create_station_table_string();
    std::string make_create_observation_table_string();
    std::string make_insert_station_table_string(); 
    std::string make_insert_observation_table_string(); 
    bool ensure_tables();
};

