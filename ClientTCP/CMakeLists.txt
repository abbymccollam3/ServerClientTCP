
set(SRCROOT ${PROJECT_SOURCE_DIR}/examples/ClientTCP)

# all source files
set(SRC ${SRCROOT}/Lab5Client.cpp
        ${SRCROOT}/TCP.cpp
        ${SRCROOT}/UDP.cpp)

# define the sockets target
sfml_add_example(ClientTCP
                 SOURCES ${SRC}
                 DEPENDS sfml-network -pthread)
