OBJS = dnstool.o db.o domain_entry.o
FLAGS = -Wall -I/usr/include/mysql
LIBS = -lldns -lmysqlpp

dnstool: $(OBJS)
	g++ $(FLAGS) $(LIBS) -o dnstool $(OBJS)

dnstool.o: dnstool.cc dnstool.h
	g++ $(FLAGS) -c dnstool.cc

db.o: db.cc db.h
	g++ $(FLAGS) -c db.cc

domain_entry.o: domain_entry.cc domain_entry.h
	g++ $(FLAGS) -c domain_entry.cc

clean:
	rm -f $(OBJS) dnstool

