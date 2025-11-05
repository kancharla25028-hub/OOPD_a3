CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic
SRC = studentdb.cpp exceptions.cpp
TARGET = studentdb

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
