#ifndef DNSTOOL_H
#define DNSTOOL_H

#include <stdio.h>

#include "db.h"

const int timelookup(std::string domain);
const std::string gen_prefix();
void got_signal(int);

#endif // DNSTOOL_H

