

#include <iostream>
#include <sstream>
#include <memory>
#include "logger.h"

#include <mariadb/conncpp.hpp>

class Db
{
public:
    Db(std::shared_ptr<std::map<std::string, std::string>> db_config);

    bool put_station_record (std::map<std::string, std::variant<std::string, float>>*);

    std::tuple<bool, std::string>
    put_observation (std::shared_ptr<std::map<std::string, std::variant<std::string, float>>>);

private:
    std::shared_ptr<std::map<std::string, std::string>> m_db_config;

    std::string m_host;
    std::string m_database;
    std::string m_user;
    std::string m_pass;
    std::string m_station_table;
    std::string m_observation_table;
    bool m_db_connected = false;


    //bool get_connection();
    bool ensure_tables();
};

