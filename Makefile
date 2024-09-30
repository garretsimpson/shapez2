ALL : display.exe search4.exe lookup4.exe search5.exe lookup5.exe

display.exe : display.cpp shapez.hpp
	g++ -o display display.cpp -static -std=c++23 -O3

search4.exe : search.cpp shapez.hpp
	g++ -o search4 search.cpp -static -std=c++23 -O3

lookup4.exe : lookup.cpp shapez.hpp
	g++ -o lookup4 lookup.cpp -static -std=c++23 -O3

search5.exe : search.cpp shapez.hpp
	g++ -o search5 search.cpp -static -std=c++23 -O3 -DCONFIG_LAYER=5

lookup5.exe : lookup.cpp shapez.hpp
	g++ -o lookup5 lookup.cpp -static -std=c++23 -O3 -DCONFIG_LAYER=5

clean:
	del display.exe search4.exe lookup4.exe search5.exe lookup5.exe
