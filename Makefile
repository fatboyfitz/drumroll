EXE=drumroll
OBJS=usb_utils.o drumroll.o alsamidi.o
CC=gcc
CFLAGS=
INCLUDES=/usr/include/SDL
LIBS=-lusb -lSDL_mixer -lasound

${EXE}: ${OBJS} 
	${CC} ${CFLAGS} `sdl-config --cflags --libs` ${LIBS} ${OBJS} -o ${EXE}

%.o: %.c
	${CC} ${CFLAGS} -I${INCLUDES} -c $<

clean:
	rm -f ${EXE} *.o 

install: drumroll
	cp ./drumroll /usr/bin/drumroll
