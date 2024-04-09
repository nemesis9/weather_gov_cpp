
#include "db.h"


/* Db constructor
 *
 * Arguments: db config
 *
 * Throws std::runtime_error
 *     if config is incomplete
*/
Db::Db(const std::map<std::string, std::string>& db_config) : m_db_config(db_config)
{

    m_host = m_port = m_database = m_user = m_pass = m_station_table = m_observation_table = "";
    for (auto const& db_item : db_config)
    {
        if (db_item.first == "host") {
            m_host = db_item.second;
        } else if (db_item.first == "port") {
            m_port = db_item.second;
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

    if ((m_host == "") || (m_port == "") || (m_database == "") || (m_user == "") ||
        (m_pass == "") || (m_station_table == "") || (m_observation_table == "")) {
        throw std::runtime_error("DB: Config is incomplete. One or more config items is missing");
    }
    ensure_tables();
}


//* PRIVATE
//
/* Name: make_create_station_table_string
 *
 * Arguments: none
 *
 * creates the string argument for creating the station table
 *
*/
std::string
Db::make_create_station_table_string() {
    std::ostringstream stringStream;
    stringStream <<  "CREATE TABLE IF NOT EXISTS " << m_station_table << " (call_id VARCHAR(5) PRIMARY KEY, name VARCHAR(80), latitude_deg FLOAT, longitude_deg FLOAT, elevation_m FLOAT,url VARCHAR(80))";
    return stringStream.str();
    //std::string copyOfStr = stringStream.str();
    //return copyOfStr;
}

/* Name: make_create_observation_table_string
 *
 * Arguments: none
 *
 * creates the string argument for creating the observation table
 *
*/
std::string
Db::make_create_observation_table_string() {
    std::ostringstream stringStream;
    stringStream << "CREATE TABLE IF NOT EXISTS " << m_observation_table << " (station_id VARCHAR(20), timestamp_UTC VARCHAR(40), temperature_C FLOAT, temperature_F FLOAT, dewpoint_C FLOAT, dewpoint_F FLOAT, description VARCHAR(40), wind_dir FLOAT, wind_spd_km_h FLOAT, wind_spd_mi_h FLOAT, wind_gust_km_h FLOAT, wind_gust_mi_h FLOAT, baro_pres_pa FLOAT, baro_pres_inHg FLOAT, rel_humidity FLOAT, PRIMARY KEY (station_id, timestamp_UTC))";
    return stringStream.str();
}

/* Name: ensure tables
 *
 * Arguments: none
 *
 * make sure the station table and the
 *     observation table exist if not
 *     there already
 *
*/
bool
Db::ensure_tables() {

    sql::Statement *stmt = nullptr;
    sql::Connection *conn = nullptr;
    try {
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":" + m_port + "/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", m_user}, {"password", m_pass}});
        conn = driver->connect(url, properties);

        std::string create_station_table_str = make_create_station_table_string();

        stmt = conn->createStatement();
        stmt->execute(create_station_table_str);

        std::string create_observation_table_str = make_create_observation_table_string();
        stmt->execute(create_observation_table_str);

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

/* Name: make_insert_station_table_string
 *
 * Arguments: none
 *
 * creates the query string for inserting
 *     a station into the station table
 *
*/
std::string
Db::make_insert_station_table_string() {
    std::ostringstream stringStream;
    stringStream << "REPLACE INTO " << m_station_table << " (call_id, name, latitude_deg, longitude_deg, elevation_m, url) VALUES (?, ?, ?, ?, ?, ?)";
    return stringStream.str();
}

/* Name: make_insert_observation_table_string
 *
 * Arguments: none
 *
 * creates the query string for inserting
 *     a observation into the observation table
 *
*/
std::string
Db::make_insert_observation_table_string() {
    std::ostringstream stringStream;
    stringStream << "INSERT INTO " << m_observation_table << " (station_id,"
            "timestamp_UTC, temperature_C, temperature_F, dewpoint_C,"
            "dewpoint_F, description, wind_dir, wind_spd_km_h, wind_spd_mi_h,"
            "wind_gust_km_h, wind_gust_mi_h, baro_pres_pa, baro_pres_inHg,"
            "rel_humidity) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    return stringStream.str();
}


//* PUBLIC

/* Name: put_station_record
 *
 * Arguments: station record
 *
 * inserts a station record into the database
 *
*/
bool
Db::put_station_record(std::map<std::string, std::variant<std::string, float>>& station_record) {
    try {

        wlog(logDEBUG) << "put station record: establishing connection";
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":" + m_port + "/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", m_user}, {"password", m_pass}});
        sql::Connection *conn = driver->connect(url, properties);

        sql::PreparedStatement  *prep_stmt;
        std::string insert_station_table_str = make_insert_station_table_string();
        prep_stmt = conn->prepareStatement(insert_station_table_str);

        prep_stmt->setString(1, std::get<std::string>(station_record["call_id"]));
        prep_stmt->setString(2, std::get<std::string>(station_record["name"]));
        prep_stmt->setFloat(3, std::get<float>(station_record["latitude_deg"]));
        prep_stmt->setFloat(4, std::get<float>(station_record["longitude_deg"]));
        prep_stmt->setFloat(5, std::get<float>(station_record["elevation_m"]));
        prep_stmt->setString(6, std::get<std::string>(station_record["url"]));

        prep_stmt->execute();
        delete prep_stmt;
        delete conn;

    } catch (sql::SQLException& e) {
        wlog(logERROR) << "Exception inserting station: " << e.what();
    }

    return true;
}


//* PUBLIC

/* Name: put_observation
 *
 * Arguments: observation record
 *
 * inserts a station observation record into the database
 *
*/

std::tuple<bool, std::string>
Db::put_observation (std::map<std::string, std::variant<std::string, float>>& obs) {

    sql::Connection *conn = nullptr;
    sql::PreparedStatement *prep_stmt = nullptr;
    try {
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::string connect_info = "jdbc:mariadb://" + m_host + ":" + m_port + "/" + m_database;
        sql::SQLString url(connect_info);
        sql::Properties properties({{"user", m_user}, {"password", m_pass}});
        conn = driver->connect(url, properties);

        sql::PreparedStatement  *prep_stmt;
        std::string insert_observation_table_str = make_insert_observation_table_string();
        prep_stmt = conn->prepareStatement(insert_observation_table_str);

        prep_stmt->setString(1, std::get<std::string>(obs["station_id"]));
        prep_stmt->setString(2, std::get<std::string>(obs["timestamp_UTC"]));
        prep_stmt->setFloat(3, std::get<float>(obs["temperature_C"]));
        prep_stmt->setFloat(4, std::get<float>(obs["temperature_F"]));
        prep_stmt->setFloat(5, std::get<float>(obs["dewpoint_C"]));
        prep_stmt->setFloat(6, std::get<float>(obs["dewpoint_F"]));
        prep_stmt->setString(7, std::get<std::string>(obs["description"]));
        prep_stmt->setFloat(8, std::get<float>(obs["wind_dir"]));
        prep_stmt->setFloat(9, std::get<float>(obs["wind_spd_km_h"]));
        prep_stmt->setFloat(10, std::get<float>(obs["wind_spd_mi_h"]));
        prep_stmt->setFloat(11, std::get<float>(obs["wind_gust_km_h"]));
        prep_stmt->setFloat(12, std::get<float>(obs["wind_gust_mi_h"]));
        prep_stmt->setFloat(13, std::get<float>(obs["baro_pres_pa"]));
        prep_stmt->setFloat(14, std::get<float>(obs["baro_pres_inHg"]));
        prep_stmt->setFloat(15, std::get<float>(obs["rel_humidity"]));

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


