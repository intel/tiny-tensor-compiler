include(CommonOptions)

find_package(SYCL REQUIRED)

set(examples main main2 main3 main4)

foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
    add_sycl_to_target(TARGET ${example} SOURCES ${example}.cpp)
    target_link_libraries(${example} PRIVATE tinytc tinytc_sycl argparser)
endforeach()
