G++ = g++
CFLAGS= -lpthread -std=c++11 -O3
LIBS = 

# App name
APPNAME = TP1_Recsys 

#Object
OBJS = main.cpp CSVReader.cpp StringUtils.cpp
release : ; $(G++) $(OBJS) -o $(APPNAME) $(LIBS) $(CFLAGS)

clean :
	rm -f $(APPNAME) *.o


