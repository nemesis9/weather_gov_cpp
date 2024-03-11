
#include <mariadb/conncpp.hpp>
#include "db.h"

extern loglevel_e wloglevel;

Db::Db(std::shared_ptr<std::map<std::string, std::string>> db_config) : m_db_config(db_config)
{
    wlog(logINFO) << "Db constructor";
    for (auto const& db_item : (*db_config))
    {
        //std::cout << db_item.first  // string (key)
        //      << ':' 
        //      << db_item.second // string's value 
        //      << std::endl;
        wlog(logINFO) << "Key: " << db_item.first << " Value: " << db_item.second;
        if (db_item.first == "host") {
            m_host = db_item.second;    
        } else if (db_item.first == "database") {
            m_database = db_item.second;
        } else if (db_item.first == "user") {
            m_user = db_item.second;
        } else if (db_item.first == "password") {
            m_pass = db_item.second;
        } else if (db_item.first == "station_table") {
            m_station_table = db_item.second;
        } else if (db_item.first == "observation_table") {
            m_observation_table = db_item.second;
        } else {
          wlog(logWARNING) << "Unknown db key: " << db_item.first;
        }
    } 
}


bool Db::ensure_database() {
    sql::Driver* driver = sql::mariadb::get_driver_instance();

    return true;
}

/*
def ensure_station_table(self) -> bool:
    try:
        cursor = self.db.cursor()
        create_table_query = """
        CREATE TABLE IF NOT EXISTS station (
            call_id VARCHAR(5) PRIMARY KEY,
            name VARCHAR(40),
            latitude_deg FLOAT,
            longitude_deg FLOAT,
            elevation_m FLOAT,
            url VARCHAR(80)
        )
        """
        cursor.execute(create_table_query)
        self.log.info("Table 'station' created successfully")
        cursor.close()
        return True
    except Exception as e:
        self.log.error(f"Exception creating station table: {e}")
        cursor.close()
        return False

def ensure_observation_table(self) -> bool:
    try:
        cursor = self.db.cursor()
        create_table_query = """
        CREATE TABLE IF NOT EXISTS observation (
            station_id VARCHAR(20),
            timestamp_UTC VARCHAR(40),
            temperature_C FLOAT,
            temperature_F FLOAT,
            dewpoint_C FLOAT,
            dewpoint_F FLOAT,
            description VARCHAR(40),
            wind_dir VARCHAR(10),
            wind_spd_km_h FLOAT,
            wind_spd_mi_h FLOAT,
            wind_gust_km_h FLOAT,
            wind_gust_mi_h FLOAT,
            baro_pres_pa FLOAT,
            baro_pres_inHg FLOAT,
            rel_humidity FLOAT,
            PRIMARY KEY (station_id, timestamp_UTC)
        );
        """
        cursor.execute(create_table_query)
        self.log.info("Table 'observation' created successfully")
        cursor.close()
        return True
    except Exception as e:
        self.log.error(f"Exception creating observation table: {e}")
        cursor.close()
        return False

*/
