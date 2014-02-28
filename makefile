CFLAGS :=-g -Wall -export-dynamic
CXXFLAGS :=-g -Wall -export-dynamic
C_SOURCES := main.c password.c mainmenu.c window_switcher.c newtrans.c RNG.c history.c settlement.c option.c registration.c config.c crypto.c
CXX_SOURCES_1 := picc_emulation_read.cpp
CXX_SOURCES_2 := picc_emulation_write.cpp
C_EXECUTABLE=nproto
CXX_EXECUTABLE_1=picc_emulation_read
CXX_EXECUTABLE_2=picc_emulation_write

all: $(C_EXECUTABLE) $(CXX_EXECUTABLE_1) $(CXX_EXECUTABLE_2)

$(C_EXECUTABLE): $(C_SOURCES)
	$(CC) $(CFLAGS) -o $(C_EXECUTABLE) $(C_SOURCES) `pkg-config gtk+-3.0 libglade-2.0 --cflags --libs` `pkg-config --cflags --libs libconfig` -lssl -lcrypto

$(CXX_EXECUTABLE_1): $(CXX_SOURCES_1)
	$(CXX) $(CXX_FLAGS) -o $(CXX_EXECUTABLE_1) $(CXX_SOURCES_1) -lCVAPIV01_DESFire
	
$(CXX_EXECUTABLE_2): $(CXX_SOURCES_2)
	$(CXX) $(CXX_FLAGS) -o $(CXX_EXECUTABLE_2) $(CXX_SOURCES_2) -lCVAPIV01_DESFire
	
clean:
	rm -rf *o  $(EXECUTABLE)






