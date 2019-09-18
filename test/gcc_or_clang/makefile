OPTS := -std=c++17 -I../ -I../../lib -o mledc_test -O3 -Wall
SOURCES := ../main.cpp ../speedtest.cpp ../noisetest.cpp ../noisetest2.cpp
HEADERS := ../speedtest.h ../noisetest.h ../noisetest2.h
LIBS=../../lib/crc32.hpp ../../lib/mledc.hpp ../../lib/fletcher32.hpp ../../lib/fletcher4.hpp

TARGET := mledc_test
EXOPTS := -mtune=native

$(TARGET) : $(SOURCES) $(LIBS) $(HEADERS)
	$(CXX) $(OPTS) $(EXOPTS) $(SOURCES)

clean:
	-rm -f $(TARGET)
