cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/CMakePCHCompiler)
project(gsc CXX CXXPCH)

set(SOURCE_FILES
        src/gsc.C
        src/Application_controller.C
        src/Navigate.h
        src/model/specializations.h
        src/model/auth/Auth_token.C
        src/model/auth/User.C
        src/model/auth/User_database.C
        src/model/game/Dictionary.C
        src/model/game/User_stats.C
        src/model/Abstract_evaluation.h
        src/model/Assignment.C
        src/model/Eval_item.C
        src/model/File_data.C
        src/model/File_meta.C
        src/model/Grader_eval.C
        src/model/Partner_request.C
        src/model/Permalink.C
        src/model/Self_eval.C
        src/model/Session.C
        src/model/Submission.C
        src/view/Abstract_file_view.C
        src/view/Accelerator_button.C
        src/view/Accelerator_text.C
        src/view/Admin_response_widget.C
        src/view/Assignments_view.C
        src/view/Admin_view.C
        src/view/Base_eval_item_widget.C
        src/view/Confirmation_dialog.C
        src/view/Date_time_edit.C
        src/view/Edit_assignment_view.C
        src/view/Error_view.C
        src/view/File_list_widget.C
        src/view/File_manager_view.C
        src/view/File_viewer_widget.C
        src/view/Held_back_view.C
        src/view/Grading_view.C
        src/view/Main_view.C
        src/view/User_suggester.C
        src/view/Eval_item_widget.C
        src/view/Evaluation_view.C
        src/view/Response_widget.C
        src/view/Submission_owner_widget.C
        src/view/Submissions_view.C
        src/view/Widget_factory.C
        src/view/game/HangmanWidget.C
        src/view/game/HighScoresWidget.C
        src/view/game/ImagesWidget.C
        src/view/game/LettersWidget.C
        src/view/game/WordWidget.C)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=gnu++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=gnu++14")

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(LIBRARIES
    wt wtdbo wtdbopostgres # wtdbosqlite3
    ${Boost_LIBRARIES})

add_executable(gsc ${SOURCE_FILES})
set_property(TARGET gsc PROPERTY CXX_STANDARD 14)
set_property(TARGET gsc PROPERTY CXX_STANDARD_REQUIRED On)
target_precompiled_header(gsc src/prefix.h)
target_link_libraries(gsc wthttp ${LIBRARIES})

add_executable(gsc-fcgi ${SOURCE_FILES})
set_property(TARGET gsc-fcgi PROPERTY CXX_STANDARD 14)
set_property(TARGET gsc-fcgi PROPERTY CXX_STANDARD_REQUIRED On)
target_link_libraries(gsc-fcgi wtfcgi ${LIBRARIES})
