EXE=drumroll
OBJS=usb_utils.o drumroll.o alsamidi.o #jackmidi.o
CC=gcc
CFLAGS=-g
INCLUDES=/usr/include/SDL
LIBS=-lusb -lSDL_mixer -lasound
SHARE_DIR=/usr/share/drumroll
BIN_DIR=/usr/bin

${EXE}: ${OBJS} 
	${CC} ${CFLAGS} `sdl-config --cflags --libs` ${LIBS} ${OBJS} -o ${EXE}

%.o: %.c
	${CC} ${CFLAGS} -I${INCLUDES} -c $<

clean:
	rm -f ./${EXE} ./*.o 

install: ${EXE}
	mkdir -p ${SHARE_DIR}
	cp -r ./samples ${SHARE_DIR} 
	cp ./${EXE} ${BIN_DIR} 
