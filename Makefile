CXX = clang++
CXXFLAGS = -Wall -Wextra -I/opt/homebrew/Cellar/lame/3.100/include
LDFLAGS = -framework CoreAudio -framework AudioToolbox -L/opt/homebrew/Cellar/lame/3.100/lib -lmp3lame

TARGET = audio_capture
RECORD = recording.mp3
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(RECORD)

.PHONY: all clean 