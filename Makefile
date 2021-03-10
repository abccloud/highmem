
TARGET  := libhighmemo.a

CFLAGS := -Iinclude -Iuthash/include -g
LDFLAGS := 

SRC_PATH = src
OBJ_PATH = src
SOURCES += $(patsubst %.c,%.c,$(notdir $(wildcard $(SRC_PATH)/*.c)))
OBJS += $(addprefix $(OBJ_PATH)/, $(SOURCES:.c=.o))

.PHONY : clean

${TARGET}:${OBJS}
	ar rcs $@  ${OBJS}

clean:
	rm -rf ${TARGET} ${OBJS} bintest

bintest:${TARGET} test.c
	${CC} ${CFLAGS} test.c $<  -o bintest
