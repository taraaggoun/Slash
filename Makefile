CC ?= gcc
INCL= -Iinclude
CCFLAGS ?= -Wall -c
CCO= $(CC) $(CCFLAGS) $(INCL) -c $< -o $@
OBJECTS= obj/slash.o obj/util.o obj/tab.o obj/command.o obj/intern_command.o obj/jokers.o obj/redirection.o

TARGET= slash
BUILD_DIR= obj

all: directory $(TARGET)

directory:
	mkdir -p $(BUILD_DIR)

memory: $(TARGET) $(OBJECTS)
	valgrind ./$(TARGET)

slash: $(OBJECTS)
	$(CC) -o slash $(OBJECTS) -lreadline

obj/slash.o: src/slash.c
	$(CCO)

obj/util.o: src/util.c
	$(CCO)

obj/tab.o: src/tab.c
	$(CCO)
	
obj/command.o: src/command.c
	$(CCO)

obj/intern_command.o: src/intern_command.c
	$(CCO)

obj/jokers.o : src/jokers.c
	$(CCO)

obj/redirection.o : src/redirection.c
	$(CCO)

clean:
	rm -rf $(TARGET) $(BUILD_DIR)