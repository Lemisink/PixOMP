cmake_minimum_required( VERSION 4.2.1 )
project(testproj)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_library( multithredor_lib
    src/image_process.cpp
    src/read_image.cpp
    src/write_image.cpp
    src/executor/single_executor.cpp
    src/executor/auto_executor.cpp
    src/executor/manual_executor.cpp
    src/operation/contrast_op.cpp
    src/operation/resize/bilinear_resize.cpp
    src/operation/resize/near_resize.cpp
    src/operation/resize/bicubic_resize.cpp
    src/convertor/gray_convertor.cpp
    src/convertor/rgb_convertor.cpp
    src/convertor/ycb_convertor.cpp
)

add_executable( mainexecutable
    main.cpp
)

find_package(OpenMP REQUIRED)
target_link_libraries(multithredor_lib PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(mainexecutable PRIVATE multithredor_lib)

target_include_directories(multithredor_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable AVX2 only for contrast_op.cpp (selective SIMD)
set_source_files_properties(src/operation/contrast_op.cpp PROPERTIES COMPILE_FLAGS "-mavx2")
