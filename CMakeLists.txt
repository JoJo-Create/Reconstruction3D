project(Reconstruction3D)
cmake_minimum_required(VERSION 2.8)

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

find_package(OpenCV 3.0 QUIET)

find_package(DBoW2 REQUIRED PATHS thrid_party/DBoW2/install/lib/cmake)
include_directories(${DBoW2_INCLUDE_DIRS})

find_package(Open3D REQUIRED PATHS thrid_party/Open3D/install/lib/cmake)
list(APPEND Open3D_LIBRARIES dl)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${Open3D_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Open3D_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${Open3D_EXE_LINKER_FLAGS}")

link_directories(${Open3D_LIBRARY_DIRS})
include_directories(${Open3D_INCLUDE_DIRS})
add_definitions(-DUNIX)
add_compile_options(-Wno-deprecated-declarations)
add_compile_options(-Wno-unused-result)
add_definitions(-O3)


add_library(${PROJECT_NAME} SHARED
        src/io/Reader.cpp
        src/io/Writer.cpp
        src/util/Converter.cpp
        src/util/Parser.cpp
        src/Frame.cpp

        src/FragmentMaker.cpp
        src/LocalRefiner.cpp
        src/FragmentsRegister.cpp
        src/GlobalRefiner.cpp
        src/Integrater.cpp

        src/GeometryMethods.cpp
        src/RegistrationMethods.cpp
        src/PoseGraphMethods.cpp
	   )

target_link_libraries(${PROJECT_NAME} ${Open3D_LIBRARIES}
                                      ${DBoW2_LIBRARIES})

add_executable(makeSegment src/tests/test_make_fragments.cpp)
target_link_libraries(makeSegment   ${PROJECT_NAME})

add_executable(registerSegment src/tests/test_register_fragments.cpp)
target_link_libraries(registerSegment  ${PROJECT_NAME})

add_executable(globalRefiner src/tests/test_global_refiner.cpp)
target_link_libraries(globalRefiner   ${PROJECT_NAME})

add_executable(localRefiner src/tests/test_local_refiner.cpp)
target_link_libraries(localRefiner   ${PROJECT_NAME})

add_executable(integrater src/tests/test_integrater.cpp)
target_link_libraries(integrater   ${PROJECT_NAME})

add_executable(reconstruct src/Reconstruction.cpp)
target_link_libraries(reconstruct  ${PROJECT_NAME})

add_executable(dbow src/tests/test_DBoW.cpp)
target_link_libraries(dbow  ${PROJECT_NAME} ${OpenCV_LIBS})



