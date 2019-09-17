OPTS := -std=c++17 -I../ -I../../lib -o mledc_test -O3
SOURCES := ../main.cpp ../speedtest.cpp ../noisetest.cpp
LIBS=../../lib/crc32.hpp ../../lib/mledc.hpp ../../lib/fletcher32.hpp ../../lib/fletcher4.hpp

TARGET := mledc_test
EXOPTS := -mtune=native

$(TARGET) : $(SOURCES) $(LIBS)
	$(CXX) $(OPTS) $(EXOPTS) $(SOURCES)

clean:
	-rm -f $(TARGET)
