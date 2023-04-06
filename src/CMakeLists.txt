find_package(cppzmq)

add_subdirectory(libraries)
add_subdirectory(drivers)

add_executable(ron_core 
    main.cpp
    joystick.cpp
    IMUReader.cpp
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
    cppzmq
)

