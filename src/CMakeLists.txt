
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

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})



find_package(PythonLibs EXACT 3.8 REQUIRED)
include_directories (${PYTHON_INCLUDE_DIRS})

add_subdirectory(libraries)
add_subdirectory(drivers)

pybind11_add_module( SDataLib MODULE
            sdata_py.cpp
            ronThread.cpp
            )

if(BUILD_FOR_SIM)
    add_executable(ron_core 
        main.cpp
        joystick.cpp
        controller.cpp
        DriveSystem.cpp
        FSM.cpp
        loggerThreaded.cpp
        ronThread.cpp
        DrakeReader.cpp
        sdata_py.cpp
    )

    target_link_libraries(ron_core 
        gpiod
        BNO055
        ODriveCAN
        MiniPID
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${PYTHON_LIBRARIES}
        msgpack11
    )
else()
    add_executable(ron_core 
        main.cpp
        joystick.cpp
        controller.cpp
        DriveSystem.cpp
        FSM.cpp
        loggerThreaded.cpp
        ronThread.cpp
        ZEDReader.cpp
        sdata_py.cpp
    )

    target_link_libraries(ron_core 
        gpiod
        BNO055
        ODriveCAN
        MiniPID
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${ZED_LIBS}
        ${PYTHON_LIBRARIES}
        msgpack11
    )
endif()

install(TARGETS SDataLib
        COMPONENT python
        LIBRARY DESTINATION "lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin")