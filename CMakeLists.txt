cmake_minimum_required(VERSION 3.20)

project(MultimediaDataProcessing LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    add_compile_options(/W3 /permissive-)
else()
    add_compile_options(-Wall)
endif()

# Programs
add_executable(sort_int     sort_int/main.cpp)
add_executable(frequencies  frequencies/main.cpp)
add_executable(binary_rw    binary_rw/main.cpp)
add_executable(bit_rw       bit_rw/main.cpp)
add_executable(inheritance  inheritance/main.cpp inheritance/canvas.cpp)
add_executable(pam          pam/main.cpp pam/pam.cpp)
add_executable(huffdiff     huffdiff/main.cpp huffdiff/pam.cpp)
add_executable(adam7        adam7/main.cpp)
add_executable(torrent_dump torrent_dump/main.cpp)
add_executable(qoa          qoa/main.cpp)

# Earlier iterations of sort_int and frequencies, kept to show how the code evolved.
option(MDP_BUILD_VERSIONS "Also build the intermediate versions in */versions" ON)

if(MDP_BUILD_VERSIONS)
    set(SORT_INT_VERSIONS
        01_without_vector_object.c
        02_with_vector_object.c
        03_vector_pointer.cpp
        04_copy_assign_tests.cpp
        05_move_const_before_copyswap.cpp
        06_move_copyswap.cpp
        07_namespaces_adl_constop_initialization.cpp
        09_std.cpp
    )
    foreach(src IN LISTS SORT_INT_VERSIONS)
        string(SUBSTRING ${src} 0 2 n)
        add_executable(sort_int_v${n} sort_int/versions/${src})
        set_target_properties(sort_int_v${n} PROPERTIES FOLDER versions)
    endforeach()

    add_executable(sort_int_v08
        sort_int/versions/08_defaultparams_templates_library/main.cpp
        sort_int/versions/08_defaultparams_templates_library/vector.cpp
    )
    set_target_properties(sort_int_v08 PROPERTIES FOLDER versions)

    add_executable(frequencies_v01 frequencies/versions/01_casting_reading_bytes.cpp)
    set_target_properties(frequencies_v01 PROPERTIES FOLDER versions)
endif()

# Round-trip tests on the sample files: run with ctest
enable_testing()

set(TEST_DIR ${CMAKE_BINARY_DIR}/test_data)
file(COPY
    huffdiff/frog.pam
    adam7/rana.pgm
    bit_rw/file01.txt
    torrent_dump/sample.torrent
    DESTINATION ${TEST_DIR}
)

# add_steps(<name> <command...> [THEN <command...>]...)
# Every command becomes a test that only runs if the previous one passed.
function(add_steps name)
    set(i 0)
    set(cmd "")
    foreach(arg IN LISTS ARGN ITEMS THEN)
        if(arg STREQUAL "THEN")
            add_test(NAME ${name}_${i} COMMAND ${cmd} WORKING_DIRECTORY ${TEST_DIR})
            set_tests_properties(${name}_${i} PROPERTIES FIXTURES_SETUP ${name}_${i})
            if(i GREATER 0)
                math(EXPR prev "${i} - 1")
                set_tests_properties(${name}_${i} PROPERTIES FIXTURES_REQUIRED ${name}_${prev})
            endif()
            math(EXPR i "${i} + 1")
            set(cmd "")
        else()
            list(APPEND cmd ${arg})
        endif()
    endforeach()
endfunction()

add_steps(huffdiff
    huffdiff c frog.pam frog.hd THEN
    huffdiff d frog.hd frog_decoded.pam THEN
    ${CMAKE_COMMAND} -E compare_files frog.pam frog_decoded.pam
)

# The input has a comment in its header, so compare the re-encoded files instead
add_steps(adam7
    adam7 c rana.pgm rana.mlt THEN
    adam7 d rana.mlt rana THEN
    adam7 c rana_7.pgm rana_again.mlt THEN
    ${CMAKE_COMMAND} -E compare_files rana.mlt rana_again.mlt
)

add_steps(bit_rw
    bit_rw w file01.txt packed.bin THEN
    bit_rw r packed.bin unpacked.txt THEN
    bit_rw w unpacked.txt packed_again.bin THEN
    ${CMAKE_COMMAND} -E compare_files packed.bin packed_again.bin
)

add_steps(binary_rw
    binary_rw w file01.txt raw.bin THEN
    binary_rw r raw.bin raw.txt THEN
    binary_rw w raw.txt raw_again.bin THEN
    ${CMAKE_COMMAND} -E compare_files raw.bin raw_again.bin
)

add_steps(torrent_dump
    torrent_dump sample.torrent
)
