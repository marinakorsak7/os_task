# Compiler to be used
COMPILER = gcc

# Compiler flags for enabling warnings
COMPILER_FLAGS = -Wall -Wextra -pedantic

# Linker flags for specifying runtime library search paths
LINK_FLAGS = -Wl,-rpath,.

# List of targets to be built
BUILD_TARGETS = client server libkv.so

# Default target: build all the specified targets
all: $(BUILD_TARGETS)

# Build the client executable
# Dependencies: client.o and libkv.so
client: client.o libkv.so
	# Linking client executable with kv library
	$(COMPILER) $(LINK_FLAGS) -o client client.o -L. -lkv

# Build the server executable
# Dependencies: server.o and libkv.so
server: server.o libkv.so
	# Linking server executable with kv library
	$(COMPILER) $(LINK_FLAGS) -o server server.o -L. -lkv

# Create the shared library libkv.so
# Dependencies: kv_lib.o
libkv.so: kv_lib.o
	# Creating the shared library
	$(COMPILER) -shared -o libkv.so kv_lib.o

# Compile client.c to client.o
# Dependencies: client.c and kv_lib.h
client.o: client.c kv_lib.h
	# Compiling client.c with specified flags
	$(COMPILER) $(COMPILER_FLAGS) -c client.c

# Compile server.c to server.o
# Dependencies: server.c and kv_lib.h
server.o: server.c kv_lib.h
	# Compiling server.c with specified flags
	$(COMPILER) $(COMPILER_FLAGS) -c server.c

# Compile kv_lib.c to kv_lib.o
# Dependencies: kv_lib.c and kv_lib.h
kv_lib.o: kv_lib.c kv_lib.h
	# Compiling kv_lib.c with specified flags
	$(COMPILER) $(COMPILER_FLAGS) -c kv_lib.c

# Clean target: remove all generated files
clean:
	# Removing object files and target executables
	rm -f *.o $(BUILD_TARGETS)

# Indicate that 'all' and 'clean' are not actual files
.PHONY: all clean
