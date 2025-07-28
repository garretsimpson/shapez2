APPS := cros display lookup ros search solve solver test analyze5 cros5 display5 lookup5 ros5 search5 solve5 test5
FILES := $(addprefix bin\,$(addsuffix .exe,$(APPS)))
# CFLAGS := -static -std=c++23 -g -Wall
CFLAGS := -static -std=c++23 -O3

all: $(FILES)

bin\cros.exe : src\ros.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS) -DCROS=true

bin\cros5.exe : src\ros.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS) -DCROS=true -DCONFIG_LAYER=5

bin\solver.exe : src\solver.cpp src\shapez.hpp src\spu.hpp
	g++ -o $@ $< $(CFLAGS)

bin\\%5.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS) -DCONFIG_LAYER=5

bin\\%.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS)

clean:
	del $(FILES)
