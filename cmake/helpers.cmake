cmake_minimum_required(VERSION 3.15)

macro(add_program target)
    add_executable(${ARGV})
    set_target_properties(${target} PROPERTIES
            CXX_STANDARD            17
            CXX_STANDARD_REQUIRED   On
            CXX_EXTENSIONS          Off)
endmacro(add_program)

set(GSC_CSS_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/server_root/html/css)

function(target_uses_css target)
    add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND                 make gsc.css
            WORKING_DIRECTORY       ${GSC_CSS_DIRECTORY}
            COMMENT                 "Building gsc.css")
endfunction(target_uses_css)

