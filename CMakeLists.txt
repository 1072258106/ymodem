PROJECT( ymodem )

INCLUDE_DIRECTORIES(
	./include
)

SET( SRC ./ymodem.c )
SET( DEMO_SRC ./demo/demo.c )

ADD_LIBRARY( ymodem ${SRC} )

ADD_EXECUTABLE( ymodem_demo ${DEMO_SRC} )
TARGET_LINK_LIBRARIES( ymodem_demo ymodem )
