#Name: Ben Temming 
#Student ID: 52094132

#Use GNU Compiler Collection (GCC)
CC=gcc

#Compiler flags (include the current directory for header files)
CFLAGS=-I.

#Header file dependencies 
DEPS = filesys.h

#Object files to be created
OBJ = shell.o filesys.o

#Rule to build object file from source files
%.o: %.c $(DEPS)
	 $(CC) -c -o $@ $< $(CFLAGS)

#Rule to build the "shell" executable and remove object files after successful build
shell: $(OBJ)
	 $(CC) -o $@ $^ $(CFLAGS)
	rm -f $(OBJ)
#Define the default target (shell)
.DEFAULT_GOAL := shell
