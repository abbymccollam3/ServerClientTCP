
set(SRCROOT ${PROJECT_SOURCE_DIR}/examples/ServerTCP)

# all source files
set(SRC ${SRCROOT}/Lab5Server.cpp
        ${SRCROOT}/TCP.cpp
        ${SRCROOT}/UDP.cpp)

# define the sockets target
sfml_add_example(ServerTCP
                 SOURCES ${SRC}
                 DEPENDS sfml-network -pthread)
