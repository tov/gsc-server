set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 3.15)
project(gsc CXX)

include(cmake/helpers.cmake)
include(cmake/Findlibgit2.cmake)

include_directories(3rdparty/xxint)

add_compile_definitions(GSC_AUTH_API_KEY)
add_compile_definitions(GSC_SHOW_QUOTA)

set(MODEL_SRC_FILES
		src/common/env_var.C
        src/common/exceptions.C
        src/common/format.C
        src/common/paths.C
        src/common/Media_type_registry.C
        src/Config.C
        src/Session.C
        src/model/specializations.h
        src/model/auth/Api_key.C
        src/model/auth/Auth_token.C
        src/model/auth/Environment.C
        src/model/auth/User.C
        src/model/game/Dictionary.C
        src/model/game/User_stats.C
        src/model/Abstract_evaluation.C
        src/model/Assignment.C
        src/model/Eval_item.C
        src/model/Exam_grade.C
        src/model/File_data.C
        src/model/File_meta.C
        src/model/Grader_eval.C
        src/model/Partner_request.C
        src/model/Permalink.C
        src/model/Self_eval.C
        src/model/Submission.C
        src/model/util/Explanation_parser.C
        src/model/util/Time_stamp.C
		)

set(CLI_SRC_FILES
        ${MODEL_SRC_FILES}
        src/view/cli/Cli_base.C)

set(WWW_SRC_FILES
        ${MODEL_SRC_FILES}
        src/gscd.C
        src/Application_controller.C
        src/Navigate.h
        src/view/Resource_base.C
        src/view/Http_status.C
        src/view/api/Resource.C
        src/view/api/Request_body.C
        src/view/api/Request_handler.C
        src/view/api/Result_array.C
        src/view/api/resources.C
        src/view/files/Resource.C
        src/view/web/Confirmation_dialog.C
        src/view/web/Notification.C
        src/view/web/PageTitle.C
        src/view/web/Username_validator.C
        src/view/web/Submission_context.C
        src/view/web/view/Abstract_file_view.C
        src/view/web/view/Admin_view.C
        src/view/web/view/Assignments_view.C
        src/view/web/view/Edit_assignment_view.C
        src/view/web/view/Error_view.C
        src/view/web/view/Evaluation_view.C
        src/view/web/view/File_manager_view.C
        src/view/web/view/Grading_stats_view.C
        src/view/web/view/Grading_view.C
        src/view/web/view/Held_back_view.C
        src/view/web/view/Main_view.C
        src/view/web/view/Profile_view.C
        src/view/web/view/Submissions_view.C
        src/view/web/widget/Accelerator_button.C
        src/view/web/widget/Accelerator_text.C
        src/view/web/widget/Admin_eval_item_widget.C
        src/view/web/widget/Admin_response_widget.C
        src/view/web/widget/Api_key_widget.C
        src/view/web/widget/Auth_widget.C
        src/view/web/widget/Base_eval_item_widget.C
        src/view/web/widget/Date_time_edit.C
        src/view/web/widget/Eval_item_type_chooser.C
        src/view/web/widget/Explanation_edit_widget.C
        src/view/web/widget/Explanation_view_widget.C
        src/view/web/widget/File_list_widget.C
        src/view/web/widget/File_viewer.C
        src/view/web/widget/Glyph_button.C
        src/view/web/widget/List_eval_item_widget.C
        src/view/web/widget/Partner_notification_widget.C
        src/view/web/widget/Response_widget.C
        src/view/web/widget/Review_eval_item_widget.C
        src/view/web/widget/Self_eval_item_widget.C
        src/view/web/widget/Single_eval_item_widget.C
        src/view/web/widget/Submission_owner_widget.C
        src/view/web/widget/Unit_line_edit.C
        src/view/web/widget/User_selector.C
        src/view/web/widget/User_suggester.C
        src/view/game/HangmanWidget.C
        src/view/game/HighScoresWidget.C
        src/view/game/ImagesWidget.C
        src/view/game/LettersWidget.C
        src/view/game/WordWidget.C)

find_package(Wt REQUIRED
        COMPONENTS wt dbo dbopostgres
        OPTIONAL_COMPONENTS wthttp wtfcgi)

find_package(libgit2 REQUIRED)

set(LIBRARIES Wt::Wt Wt::Dbo Wt::DboPostgres git2)

if(TARGET Wt::HTTP)
    add_program(gscd ${WWW_SRC_FILES})
    target_link_libraries(gscd Wt::HTTP ${LIBRARIES})
    target_compile_definitions(gscd PRIVATE GSC_AUTH_DEBUG)
    target_uses_resources(gscd)
endif(TARGET Wt::HTTP)

if(TARGET Wt::FCGI)
    add_program(gscd-fcgi ${WWW_SRC_FILES})
    target_link_libraries(gscd-fcgi Wt::FCGI ${LIBRARIES})
    target_compile_definitions(gscd-fcgi PRIVATE GSC_AUTH_OPEN_AM)
    target_uses_resources(gscd-fcgi)
endif(TARGET Wt::FCGI)

add_program(gsc-auth src/gsc-auth.C ${CLI_SRC_FILES})
target_link_libraries(gsc-auth ${LIBRARIES})

add_program(gsc-createdb src/gsc-createdb.C ${CLI_SRC_FILES})
target_link_libraries(gsc-createdb ${LIBRARIES})

