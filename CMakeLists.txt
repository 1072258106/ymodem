PROJECT( ymodem )

INCLUDE_DIRECTORIES(
	./include
)

SET( SRC 
	./ymodem.c
)

ADD_LIBRARY( ymodem ${SRC} )
