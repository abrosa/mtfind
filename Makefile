filename="./resources/hugetext.bin"
mask1="n?gger"
mask2="n.gger"

CPP=g++
CPPFLAGS=-std=c++20 -Werror
BOOST=-lboost_iostreams-mt

all: result1.diff result2.diff
	cat $^

mtfind.exe: mtfind.cpp
	$(CPP) $(CPPFLAGS) $< -o $@ $(BOOST)

mtfind.out: mtfind.exe
	./$< $(filename) $(mask1) > $@

noregex.exe: noregex.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

noregex.out: noregex.exe
	./$< $(filename) $(mask1) > $@

regex.exe: regex.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

regex.out: regex.exe
	./$< $(filename) $(mask2) > $@

result1.diff: mtfind.out noregex.out
	-diff $^ > $@

result2.diff: mtfind.out regex.out
	-diff $^ > $@

clean:
	rm -f *.out *.exe *.diff