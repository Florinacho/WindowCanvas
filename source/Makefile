OUTPUT=../bin/libwcanvas.a

ifeq ($(OS),Windows_NT)
	CC=mingw32-g++.exe
else
	CC=g++
endif

INCLUDE=
LIB_DIRS=
LIB_FILES=
C_FLAGS=-O3 -g3 -Wall -D_DEBUG
L_FLAGS=
C_FILES=WindowCanvas.cpp

C_FLAGS+=$(addprefix -I, $(INCLUDE))
L_FLAGS+=$(addprefix -L, $(LIB_DIRS)) $(addprefix -l, $(LIB_FILES)) 
L_FILES=$(C_FILES:.cpp=.o)

build: $(OUTPUT)

clean:
	rm -rf $(L_FILES) $(OUTPUT)

rebuild: clean build

%.o: %.cpp
	$(CC) $(C_FLAGS) -c $< -o $@

%.a: $(L_FILES)
	ar rvs $@ $(L_FILES) $(L_FLAGS)


.PHONY: