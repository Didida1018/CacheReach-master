CC       = g++
CPPFLAGS = -Wno-deprecated -O3 -c -m64 -std=c++11 -fopenmp
LDFLAGS  = -O3 -fopenmp
SOURCES  = main.cpp Util.cpp Graph.cpp
OBJECTS  = $(SOURCES:.cpp=.o)
EXECUTABLE = CacheReach  

.PHONY: all clean

all: $(EXECUTABLE)
	rm -f $(OBJECTS)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
