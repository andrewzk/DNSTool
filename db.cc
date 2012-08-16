#include "db.h"

DB::DB() {
}

DB::~DB() {
  for(std::map<std::string, DomainEntry*>::iterator iter = 
      domain_stats.begin(); iter != domain_stats.end(); ++iter) {
    
    DomainEntry *entry = (*iter).second;
    delete entry;
  }
}

/* Adds a new time series element into the database and updates the 
 * corresponding DomainEntry object
 */
void DB::insert_data(std::string domain, int latency) {
  /* Pull the domain_stats column */
  mysqlpp::Query pull_stats = conn.query("SELECT * FROM domain_stats " 
                                         "WHERE name = '" + domain + "'");

  try {
    if (mysqlpp::StoreQueryResult pull_stats_res = pull_stats.store()) {  
  
      if(pull_stats_res.empty()) {
        /* Create a new DomainEntry object */
        DomainEntry *new_entry = new DomainEntry(domain);    
        new_entry->add(latency);
        domain_stats[domain] = new_entry;
        
        /* Enter domain in DB */
        mysqlpp::Query query = conn.query("INSERT INTO domain_stats VALUES("
                                          "%0q, %1, %2, %3, NOW(), NOW())");
        query.parse();    
        query.store(domain.c_str(), (double)latency, 1, 0.0);
      }
      else {
        DomainEntry *existing_entry = domain_stats[domain];
        existing_entry->add(latency);
        /* Update values in DB */
        mysqlpp::Query query = conn.query("UPDATE domain_stats SET num_queries="
                                          "%0, avg_latency = %1," 
                                          "stdev = %2 WHERE name = %3q");
        query.parse();
        query.store(existing_entry->num_data_values(), existing_entry->mean(), 
              existing_entry->stdev(), domain);
      }
    }
  }
  catch(std::exception& er) {
    std::cerr << "Error executing query: " << er.what() << std::endl;
  }

  /* Create new time series entry */
  std::stringstream ss;
  std::string insert_latency = "INSERT INTO time_series VALUES('" + domain + 
                               "', NOW(), ";

  ss << insert_latency << latency;
  std::string final_query = ss.str();
  final_query += ")";
  
  try {  
    mysqlpp::Query q = conn.query(final_query);
    q.store();
  }
  catch(std::exception& er) {
    std::cerr << "Error executing query: " << er.what() << std::endl;
  }
}

/* Attempt to connect to database. Return 0 if success, -1 if failure
 * If time_series and domain_stats tables don't exist already, they are created.
 * If some rows are already in the tables, we create DomainEntry objects for 
 * them and populate their values using the time_series table
 */
int DB::connect() {
  /* Read DB info from config file */
  std::ifstream f("db.conf");
  if (!f) {
    std::cerr << "Unable to open db.conf" << std::endl;
    return -1;
  }

  std::string server, db, user, pass;
  std::string line;
  while(std::getline(f, line)) {
    std::istringstream iss(line);
    std::string key;
    std::string value;
    std::getline(iss, key, '=');
    std::getline(iss, value);

    /* Trim whitespace via stream extraction */
    std::stringstream trimmer;
    trimmer << key;
    key.clear();
    trimmer >> key;
    trimmer << value;
    value.clear();
    trimmer >> value;

    if(key.compare("host") == 0) {
      server = value;
    }
    else if(key.compare("db") == 0) {
      db = value;
    }
    else if(key.compare("user") == 0) {
      user = value;
    }
    else if(key.compare("pass") == 0) {
      pass = value;
    }
  }
  f.close();

  try {
    conn = mysqlpp::Connection(db.c_str(), server.c_str(), user.c_str(), 
                   pass.c_str(), 0);
  }
  catch(std::exception& er) {
    std::cerr << "DB Error: " << er.what() << std::endl;
    return -1;
  }

  /* Create the tables if they don't exist */
  std::string create_table1 = "CREATE TABLE IF NOT EXISTS domain_stats("
                              "name VARCHAR(50) NOT NULL, avg_latency DOUBLE, "
                              "num_queries INTEGER, stdev DOUBLE, "
                              "first_query DATETIME, "
                              "last_query TIMESTAMP, PRIMARY KEY (name))";  
  
  std::string create_table2 = "CREATE TABLE IF NOT EXISTS time_series("
                              "domain_name VARCHAR(50) NOT NULL, "
                              "time TIMESTAMP, latency INTEGER, "
                              "FOREIGN KEY (domain_name) "
                              "REFERENCES domain_stats(name))";

  mysqlpp::Query q1 = conn.query(create_table1);
  mysqlpp::Query q2 = conn.query(create_table2);  
  
  try {      
    q1.store();
    q2.store();
  }
  catch(std::exception& er) {
    std::cerr << "Error executing query: " << er.what() << std::endl;
    return -1;
  }  

  /* In case there are already some existing values in the DB upon starting, 
   * we need to create DomainEntry objects for them */
  mysqlpp::Query pull_ts_query = conn.query("SELECT * from time_series"); 
  
  try {  
    if(mysqlpp::StoreQueryResult pull_ts_res = pull_ts_query.store()) {
      for(size_t i = 0; i < pull_ts_res.num_rows(); i++) {
        mysqlpp::String s = pull_ts_res[i]["domain_name"];      
        std::string domain = std::string(s.data(), s.length());
        int latency = pull_ts_res[i]["latency"];
        if (!domain_stats.count(domain)) {
          DomainEntry *new_entry = new DomainEntry(domain);
          domain_stats[domain] = new_entry;
        }
        domain_stats[domain]->add(latency);
      }
    }
  }
  catch(std::exception& er) {
    std::cerr << "Error executing query: " << er.what() << std::endl;
    return -1;
  }

  return 0;
}
