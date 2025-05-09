CC = g++
CFLAGS = -Wall -std=c++11

SRCS = sho.cpp discreteSim.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = sho

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	rm -f $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: cleanDocs
cleanDocs:
	rm -r ./docs
