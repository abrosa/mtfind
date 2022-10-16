#OBJS specifies which files to compile as part of the project
OBJS = ./src/*.cpp

#CC specifies which compiler we're using
CC = g++

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -Iinclude -IC:/msys64/mingw64/include/boost

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:/msys64/mingw64/lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# -Werror treat all warnings as errors
# -std=c++20 use the last c++ standard
COMPILER_FLAGS = -Werror -std=c++20

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lmingw32 -lboost_iostreams-mt

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = ./release/mtfind

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
