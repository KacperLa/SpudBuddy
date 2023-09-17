find_package(cppzmq REQUIRED)
message(zppzmq:${cppzmq_VERSION})
find_package(ZED 3 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)

#set(msgpack11_DIR "/opt/code/ronTheHelper/src/libraries/msgpack11/build")

include_directories(${ZED_INCLUDE_DIRS})
include_directories(${CUDA_INCLUDE_DIRS})

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})


#SET(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})

add_subdirectory(libraries)
add_subdirectory(drivers)

add_executable(ron_core 
    main.cpp
    joystick.cpp
    IMUReader.cpp
    ZEDReader.cpp
    controller.cpp
    DriveSystem.cpp
    FSM.cpp
    Logging.cpp
    ronThread.cpp
    cmd.cpp
)

target_link_libraries(ron_core 
    gpiod
	BNO055
    ODriveCAN
    MiniPID
    ${CMAKE_THREAD_LIBS_INIT}
    ${ZED_LIBS}
    cppzmq
    msgpack11
)