APPS := display lookup search solve display5 lookup5 search5 solve5
FILES := $(addprefix bin\,$(addsuffix .exe,$(APPS)))
CFLAGS := -static -std=c++23 -O3

all: $(FILES)

bin\\%5.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS) -DCONFIG_LAYER=5

bin\\%.exe : src\%.cpp src\shapez.hpp
	g++ -o $@ $< $(CFLAGS)

clean:
	del $(FILES)
