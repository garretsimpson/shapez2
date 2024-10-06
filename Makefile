APPS := $(addprefix bin\,$(addsuffix .exe,display search lookup solver search5 lookup5 display5))
CFLAGS := -static -std=c++23 -O3

all: $(APPS)

bin\\%5.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS) -DCONFIG_LAYER=5

bin\\%.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS)

clean:
	del $(APPS)
