PROJECT( ymodem )

INCLUDE_DIRECTORIES(
	./include
	./demo/serial/include
)

SET( SRC ./ymodem.c )
SET( DEMO_SRC 
	./demo/demo.cpp 
	./demo/serial/src/serial.cc
	./demo/serial/src/impl/unix.cc
	./demo/serial/src/impl/list_ports/list_ports_osx.cc
)

if(APPLE)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(FOUNDATION_LIBRARY Foundation)
endif()


ADD_LIBRARY( ymodem ${SRC} )

ADD_EXECUTABLE( ymodem_demo ${DEMO_SRC} )
if(APPLE)
	target_link_libraries( ymodem_demo ymodem ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
endif()
