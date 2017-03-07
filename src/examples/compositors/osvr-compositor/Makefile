MOTORCAR_DIR=../../../..
CC=g++


INCLUDES= -I${MOTORCAR_DIR}/src/compositor -I${MOTORCAR_DIR}/src/protocol -I${MOTORCAR_DIR}/src/device
LIBS= -L${MOTORCAR_DIR}/lib -lmotorcar-compositor `pkg-config --libs Qt5Network Qt5Quick Qt5Widgets` -losvrClientKit
CFLAGS=-std=c++11 
objects=osvr_hmd.o

compositor: main.cpp $(objects)
	${CC}  main.cpp  ${CFLAGS} ${INCLUDES} ${LIBS} -o osvr-compositor 

osvr_hmd.o: ${MOTORCAR_DIR}/src/device/osvr_hmd.cpp ${MOTORCAR_DIR}/src/device/osvr_hmd.h
	${CC} -c ${MOTORCAR_DIR}/src/device/osvr_hmd.cpp ${CFLAGS} ${INCLUDES} ${LIBS}


clean:
	rm -f osvr-compositor $(objects)
