/* Main file for dnstool
 *
 * Usage: ./dnstool delay
 * Delay is in seconds.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <fstream>

#include <ldns/ldns.h>

#include "dnstool.h"

#define NUM_DOMAINS 10
#define LEN_PREFIX 8

const std::string domains[] = {"google.com", "facebook.com", "youtube.com",
                               "yahoo.com", "live.com", "wikipedia.org",
                               "baidu.com", "blogger.com", "msn.com", "qq.com"};

bool quit = false;

void got_signal(int) {
    quit = true;
}

/* Sends a DNS query to the specified domain and returns the query latency */
const int time_lookup(std::string domain) {
    
  ldns_resolver *res;
  ldns_rdf *domain_rdf;
  ldns_pkt *p;  
  ldns_status s;

  res = NULL;
  domain_rdf = NULL;
  p = NULL;

  domain_rdf = ldns_dname_new_frm_str(domain.c_str());
  if(!domain_rdf) {
    std::cerr << "Error creating LDNS RDF" << std::endl;
    return -1;
  }
    
  /* Create resolver from /etc/resolv.conf */
  s = ldns_resolver_new_frm_file(&res, NULL);
  if (s != LDNS_STATUS_OK) {
    std::cerr << "Error creating LDNS Resolver" << std::endl;
    return -1;
  }
    
  ldns_resolver_set_recursive(res, true);
  
  p = ldns_resolver_query(res, domain_rdf, LDNS_RR_TYPE_A, 
                          LDNS_RR_CLASS_IN, LDNS_RD);  

  ldns_rdf_deep_free(domain_rdf);

  if(!p) {
    std::cerr << "DNS error" << std::endl;
    return -1;
  }

  int query_time = ldns_pkt_querytime(p);
  std::cout << "Query time for " << domain << ": " << query_time << "ms" 
            << std::endl;
    
  ldns_pkt_free(p);
  ldns_resolver_deep_free(res);    

  return query_time;
}

/* Generates a random prefix to avoid hitting DNS cache */
const std::string gen_prefix() {
    
  static const char all_chars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    
  std::string prefix = "";    

  for(int i = 0; i < LEN_PREFIX; i++) {
    prefix += all_chars[rand() / (RAND_MAX / (sizeof(all_chars) - 1))];
  }

  return prefix;
}

int main(int argc, char *argv[]) {
    
  if(argc != 2) {
    std::cerr << "Usage: ./dnstool delay" << std::endl;
    return -1;
  }

  int delay = atoi(argv[1]);
  if (delay < 1) {
    std::cerr << "Delay must be > 0" << std::endl;
    return -1;
  }

  /* Register SIGINT handler */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = got_signal;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);

  srand(time(0));

  DB *db = new DB();
  if(db->connect() == -1) {
    delete db;
    return -1;
  }

  while(1) {  
    for(int i = 0; i < NUM_DOMAINS; i++) {
      std::string domain = gen_prefix() + "." + domains[i];
      int latency = time_lookup(domain);
      if(latency != -1)
        db->insert_data(domain.substr(LEN_PREFIX + 1), latency);
    }
    sleep(delay);
    if(quit) {
      delete db;
      break;      
    }
  }

  return 0;
}
