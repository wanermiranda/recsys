G++ = g++
CFLAGS= -std=c++11 
LIBS = -lpthread
OPTS = -O3
DEBUG = -g

# App name
APPNAME = TP1_Recsys 

#Object
OBJS = main.cpp CSVReader.cpp StringUtils.cpp
release : ; $(G++) $(OBJS) -o $(APPNAME) $(LIBS) $(CFLAGS) $(OPTS)

debug	: ; $(G++) $(OBJS) -o $(APPNAME) $(LIBS) $(CFLAGS) $(DEBUG)
clean :
	rm -f $(APPNAME) *.o


