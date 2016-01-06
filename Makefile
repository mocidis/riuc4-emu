.PHONY: all clean
APP:=riu4c-emu
TEST:=read_test
MAIN_DIR:=.
MAIN_SRC:=master.c
TEST_SRC:=reader.c
CFLAGS:=-I$(MAIN_DIR)/include
LIBS:=-lpthread

all: $(APP) $(TEST)
CROSS_PREFIX:=arm-linux-gnueabihf-
CC:=$(CROSS_PREFIX)gcc
$(APP): $(MAIN_SRC:.c=.o)
	$(CC) -o $@ $^ $(LIBS)

$(TEST): $(TEST_SRC:.c=.o)
	$(CC) -o $@ $^ $(LIBS)

$(TEST_SRC:.c=.o): %.o: $(MAIN_DIR)/src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(MAIN_SRC:.c=.o): %.o: $(MAIN_DIR)/src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -fr *.o $(APP) $(TEST)
