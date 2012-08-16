#include <math.h>

#include "domain_entry.h"

DomainEntry::DomainEntry(std::string _domain_name) {
  domain_name = _domain_name;
  m_n = 0;
  m_oldM = m_newM = m_oldS = m_newS = 0.0;
}

DomainEntry::~DomainEntry() {
}

void DomainEntry::add(int x) {
  m_n++;

  /* See Knuth TAOCP vol 2, 3rd edition, page 232 */
  if (m_n == 1) {
    m_oldM = m_newM = x;
    m_oldS = 0;
  }
  else {
    m_newM = m_oldM + (x - m_oldM)/m_n;
    m_newS = m_oldS + (x - m_oldM)*(x - m_newM);
    
    /* set up for next iteration */
    m_oldM = m_newM; 
    m_oldS = m_newS;
  }
}

int DomainEntry::num_data_values() const {
  return m_n;
}

double DomainEntry::mean() const {
  return (m_n > 0) ? m_newM : 0.0;
}

double DomainEntry::variance() const {
  return ( (m_n > 1) ? m_newS/(m_n - 1) : 0.0 );
}

double DomainEntry::stdev() const {
  return sqrt( variance() );
}

std::string DomainEntry::get_domain_name() const {
  return domain_name;
}
