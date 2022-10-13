OUTPUT=main.exe

CC=g++
#CC=mingw32-g++.exe

C_FLAGS=-O3 -g3
#-Wl,-subsystem,windows
L_FLAGS=-flto

LIBS=-lX11
#LIBS=-lgdi32

SOURCE_FILES=WindowCanvas.cpp main.cpp

OBJECT_FILES = $(SOURCE_FILES:.cpp=.o)

build: $(OUTPUT)

clean:
	rm -rf $(OBJECT_FILES) $(OUTPUT)

rebuild: clean build

%.o: %.cpp
	$(CC) $(C_FLAGS) -c $< -o $@

$(CORE_SOURCE)/%.o: $(CORE_SOURCE)/%.cpp
	$(CC) $(C_FLAGS) -c $< -o $@

$(OUTPUT): $(OBJECT_FILES)
	$(CC) $(L_FLAGS) $(OBJECT_FILES) $(LIBS) -o $(OUTPUT)
	chmod +xr $(OUTPUT)