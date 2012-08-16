/* This class maintains a MySQL++ database connection and facilitates the 
 * insertion of new time series values
 */

#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <map>

#include <mysql++/mysql++.h>

#include "domain_entry.h"

class DB {
 public:
  DB();
  ~DB();
  int connect();
  void insert_data(std::string domain, int latency);
  
 private:
  mysqlpp::StoreQueryResult* exec_query(std::string s);
  mysqlpp::StoreQueryResult* exec_query(mysqlpp::Query *q);

  mysqlpp::Connection conn;
  std::map<std::string, DomainEntry*> domain_stats;
};

#endif /* DB_H */

