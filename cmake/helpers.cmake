cmake_minimum_required(VERSION 3.15)

macro(add_program target)
    add_executable(${ARGV})
    set_target_properties(${target} PROPERTIES
            CXX_STANDARD            17
            CXX_STANDARD_REQUIRED   On
            CXX_EXTENSIONS          Off)
endmacro(add_program)

set(GSC_HTML_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/server_root/html)

function(target_uses_resources target)
    add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND                 make all
            WORKING_DIRECTORY       ${GSC_HTML_DIRECTORY}
            COMMENT                 "Building web resources")
endfunction(target_uses_resources)

