find_package(cppzmq)

add_subdirectory(libraries)
add_subdirectory(drivers)

add_executable(ron_core 
    main.cpp
    joystick.cpp
)

target_link_libraries(ron_core 
    gpiod
	BNO055
    ${CMAKE_THREAD_LIBS_INIT}
    readerwriterqueue
    cppzmq
)

