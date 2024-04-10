CFLAGS = -Wall -std=c++11 -g
HEADER=simcpp.h protothread.h
SOURCE=simcpp.cpp
EXE=example-minimal example-twocars

.PHONY: clean

all: $(EXE)

%: %.cpp $(HEADER) $(SOURCE)
	CFLAGS $< $(SOURCE) -o $@

clean:
	rm $(EXE)
