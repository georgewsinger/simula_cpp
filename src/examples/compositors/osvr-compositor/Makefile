MOTORCAR_DIR=../../../..
CC=g++


INCLUDES= -I${MOTORCAR_DIR}/src/compositor -I${MOTORCAR_DIR}/src/protocol -I${MOTORCAR_DIR}/src/device
LIBS= -L${MOTORCAR_DIR}/lib -lmotorcar-compositor `pkg-config --libs Qt5Network Qt5Quick Qt5Widgets` -losvrClientKit -losvrRenderManager
CFLAGS=-std=c++11 

compositor: main.cpp ${MOTORCAR_DIR}/src/device/osvr_hmd.cpp ${MOTORCAR_DIR}/src/device/osvr_hmd.h ${MOTORCAR_DIR}/src/device/osvr_controller.cpp ${MOTORCAR_DIR}/src/device/osvr_controller.h
	${CC}  main.cpp ${MOTORCAR_DIR}/src/device/osvr_hmd.cpp ${MOTORCAR_DIR}/src/device/osvr_hmd.h ${MOTORCAR_DIR}/src/device/osvr_controller.cpp ${MOTORCAR_DIR}/src/device/osvr_controller.h ${CFLAGS} ${INCLUDES} ${LIBS} -o osvr-compositor 

clean:
	rm -f osvr-compositor
