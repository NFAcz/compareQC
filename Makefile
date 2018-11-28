.PHONY:all

all:
	gcc -o compareQC -lm -lz -lboost_iostreams -lstdc++ main.cpp

clean:
	rm compareQC

install:
	cp compareQC /usr/local/bin
