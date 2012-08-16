/* This class uses Welford's method of computing running variance
 * Adapted from http://www.johndcook.com/standard_deviation.html 
 *
 * It's important to keep a running variance to avoid the need to pull
 * the entire timeseries from the DB for each new entry
 */

#ifndef DOMAINENTRY_H
#define DOMAINENTRY_H

#include <iostream>

class DomainEntry {

 public:
  explicit DomainEntry(std::string _domain_name);
  ~DomainEntry();
  void add(int x);
  int num_data_values() const;
  double mean() const;
  double variance() const;
  double stdev() const;
  std::string get_domain_name() const;

 private:
  std::string domain_name;
  int m_n;
  double m_oldM, m_newM, m_oldS, m_newS;
};

#endif /* DOMAINENTRY_H */

