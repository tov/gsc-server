cmake_minimum_required(VERSION 3.5)
project(gsc-css)

add_custom_command(
        OUTPUT gsc.css
        COMMAND sass gsc.scss gsc.css
        MAIN_DEPENDENCY gsc.scss
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(css ALL DEPENDS gsc.css)
