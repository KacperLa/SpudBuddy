
if(BUILD_FOR_SIM)
    message("BUILD_FOR_SIM is on")
    # Add commands to be executed when MY_CUSTOM_FLAG is on
else()
    find_package(ZED 3 REQUIRED)
    find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)

    include_directories(${ZED_INCLUDE_DIRS})
    include_directories(${CUDA_INCLUDE_DIRS})

    link_directories(${ZED_LIBRARY_DIR})
    link_directories(${CUDA_LIBRARY_DIRS})


    SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
endif()

add_subdirectory(libraries)
add_subdirectory(drivers)

if(BUILD_FOR_SIM)
    add_executable(ron_core 
        main.cpp
        joystick.cpp
        IMUReader.cpp
        controller.cpp
        DriveSystem.cpp
        FSM.cpp
        Logging.cpp
        ronThread.cpp
        DrakeReader.cpp
    )

    target_link_libraries(ron_core 
        gpiod
        BNO055
        ODriveCAN
        MiniPID
        ${CMAKE_THREAD_LIBS_INIT}
        msgpack11
    )
else()
    add_executable(ron_core 
        main.cpp
        joystick.cpp
        IMUReader.cpp
        controller.cpp
        DriveSystem.cpp
        FSM.cpp
        Logging.cpp
        ronThread.cpp
        ZEDReader.cpp
    )

    target_link_libraries(ron_core 
        gpiod
        BNO055
        ODriveCAN
        MiniPID
        ${CMAKE_THREAD_LIBS_INIT}
        ${ZED_LIBS}
        msgpack11
    )
endif()