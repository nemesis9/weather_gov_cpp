
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

    ensure_tables();
}


//bool
//Db::get_connection() {
//    wlog(logINFO) << "Db:ensure database";
//    sql::Statement *stmt;
//    try {
//        sql::Driver* driver = sql::mariadb::get_driver_instance();
//
//        wlog(logINFO) << "Db:ensure database: configuring connection";
//        // Configure connection
//        std::string connect_info = "jdbc:mariadb://" + m_host + ":3306/" + m_database;
//        sql::SQLString url(connect_info);
//        sql::Properties properties({{"user", "weather_user"}, {"password", "weather_pass"}});
//
//        wlog(logINFO) << "Db:ensure database: establishing connection";
//        //Establish connection
//        //std::unique_ptr<sql::Connection> m_conn(driver->connect(url, properties));
//        m_conn = driver->connect(url, properties);
//        stmt = m_conn->createStatement();
//        stmt->execute("SELECT * from station");
//    }
//    catch(sql::SQLException& e){
//      std::cerr << "Error selecting from station table: " << e.what() << std::endl;
//      wlog(logERROR) << "Error selecting from station table: " << e.what();
//      return false;
//   }
//
//   return true;
//}


bool
Db::ensure_tables() {

    sql::Statement *stmt = nullptr;
    sql::Connection *conn = nullptr;
    try {
        wlog(logINFO) << "ensure tables: establishing connection";
        //sql::Statement *stmt;
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":3306/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", "weather_user"}, {"password", "weather_pass"}});
        conn = driver->connect(url, properties);
        wlog(logINFO) << "ensure tables: connection established";

        stmt = conn->createStatement();
        stmt->execute("CREATE TABLE IF NOT EXISTS station_cpp (call_id VARCHAR(5) PRIMARY KEY, name VARCHAR(80), latitude_deg FLOAT, longitude_deg FLOAT, elevation_m FLOAT,url VARCHAR(80))");
        stmt->execute("SELECT * from station_cpp");

        stmt->execute("CREATE TABLE IF NOT EXISTS observation_cpp (station_id VARCHAR(20), timestamp_UTC VARCHAR(40), temperature_C FLOAT, temperature_F FLOAT, dewpoint_C FLOAT, dewpoint_F FLOAT, description VARCHAR(40), wind_dir VARCHAR(10), wind_spd_km_h FLOAT, wind_spd_mi_h FLOAT, wind_gust_km_h FLOAT, wind_gust_mi_h FLOAT, baro_pres_pa FLOAT, baro_pres_inHg FLOAT, rel_humidity FLOAT, PRIMARY KEY (station_id, timestamp_UTC))");

        delete stmt;
        delete conn;
        return true;
    } catch (sql::SQLException& e) {
        if (stmt) delete stmt;
        if (conn) delete conn;
        wlog(logERROR) << "Exception ensuring table" << e.what();
        return false;
    }

}



bool Db::put_station_record(std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> station_record) {
    wlog(logINFO) << "put station record: " << std::get<std::string>((*station_record)["name"]);
    try {

        wlog(logINFO) << "put station record: establishing connection";
        //sql::Statement *stmt;
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":3306/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", "weather_user"}, {"password", "weather_pass"}});
        sql::Connection *conn = driver->connect(url, properties);
        wlog(logINFO) << "put station record: connection established";

        sql::PreparedStatement  *prep_stmt;
        prep_stmt = conn->prepareStatement("REPLACE INTO station_cpp(call_id, name, latitude_deg, longitude_deg, elevation_m, url) VALUES (?, ?, ?, ?, ?, ?)");

        prep_stmt->setString(1, std::get<std::string>((*station_record)["call_id"]));
        prep_stmt->setString(2, std::get<std::string>((*station_record)["name"]));
        prep_stmt->setFloat(3, std::get<float>((*station_record)["latitude_deg"]));
        prep_stmt->setFloat(4, std::get<float>((*station_record)["longitude_deg"]));
        prep_stmt->setFloat(5, std::get<float>((*station_record)["elevation_m"]));
        prep_stmt->setString(6, std::get<std::string>((*station_record)["url"]));

        prep_stmt->execute();
        delete prep_stmt;
        delete conn;

    } catch (sql::SQLException& e) {
        wlog(logERROR) << "Exception inserting station: " << e.what();
    }

    return true;
}


std::tuple<bool, std::string>
Db::put_observation (std::shared_ptr<std::map<std::string, std::variant<std::string, float>>> obs) {
    wlog(logINFO) << "put station observation: ";

    sql::Connection *conn = nullptr;
    sql::PreparedStatement *prep_stmt = nullptr;
    try {
        wlog(logINFO) << "put station observation: establishing connection";
        //sql::Statement *stmt;
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":3306/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", "weather_user"}, {"password", "weather_pass"}});
        //sql::Connection *conn = driver->connect(url, properties);
        conn = driver->connect(url, properties);
        wlog(logINFO) << "put station observation: connection established";

        sql::PreparedStatement  *prep_stmt;
        prep_stmt = conn->prepareStatement("INSERT INTO observation_cpp (station_id,"
            "timestamp_UTC, temperature_C, temperature_F, dewpoint_C,"
            "dewpoint_F, description, wind_dir, wind_spd_km_h, wind_spd_mi_h,"
            "wind_gust_km_h, wind_gust_mi_h, baro_pres_pa, baro_pres_inHg,"
            "rel_humidity) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        prep_stmt->setString(1, std::get<std::string>((*obs)["station_id"]));
        prep_stmt->setString(2, std::get<std::string>((*obs)["timestamp_UTC"]));
        prep_stmt->setFloat(3, std::get<float>((*obs)["temperature_C"]));
        prep_stmt->setFloat(4, std::get<float>((*obs)["temperature_F"]));
        prep_stmt->setFloat(5, std::get<float>((*obs)["dewpoint_C"]));
        prep_stmt->setFloat(6, std::get<float>((*obs)["dewpoint_F"]));
        prep_stmt->setString(7, std::get<std::string>((*obs)["description"]));
        prep_stmt->setFloat(8, std::get<float>((*obs)["wind_dir"]));
        prep_stmt->setFloat(9, std::get<float>((*obs)["wind_spd_km_h"]));
        prep_stmt->setFloat(10, std::get<float>((*obs)["wind_spd_mi_h"]));
        prep_stmt->setFloat(11, std::get<float>((*obs)["wind_gust_km_h"]));
        prep_stmt->setFloat(12, std::get<float>((*obs)["wind_gust_mi_h"]));
        prep_stmt->setFloat(13, std::get<float>((*obs)["baro_pres_pa"]));
        prep_stmt->setFloat(14, std::get<float>((*obs)["baro_pres_inHg"]));
        prep_stmt->setFloat(15, std::get<float>((*obs)["rel_humidity"]));

        prep_stmt->execute();
        delete prep_stmt;
        delete conn;

    } catch (sql::SQLException& e) {
        if (prep_stmt) delete conn;
        if (conn) delete conn;
        wlog(logERROR) << "Exception inserting observation: " << e.what();
        return {false, std::string(e.what())};
    }

    return {true, ""};
}


/*
 *
 //try {
    //    // Create a new Statement
    //    std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
    //    // Execute query
    //    sql::ResultSet *res = stmnt->executeQuery("select * from station");
    //    // Loop through and print results
    //    while (res->next()) {
    //        std::cout << "call_id = " << res->getString(1);
    //        std::cout << ", name = " << res->getString(2);
    //        std::cout << ", latitude_deg = " << res->getFloat(3);
    //        std::cout << ", longitude_deg = " << res->getFloat(4);
    //        std::cout << ", elevation = " << res->getFloat(5);
    //        std::cout << ", url = " << res->getString(6) << "\n";
    //    }
    //}

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
