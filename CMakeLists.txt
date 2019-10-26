cmake_minimum_required(VERSION 3.5)
project(gsc CXX)

set(MODEL_SRC_FILES
        src/view/api/Endpoint.C
        src/view/api/Http_status.C
        src/common/env_var.C
        src/common/format.C
        src/common/paths.C
        src/common/Media_type_registry.C
        src/view/api/Request_body.C
        src/view/api/Request_handler.C
        src/view/api/resources.C
        src/view/api/Result_array.C
        src/model/specializations.h
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
        src/Session.C
        src/model/Submission.C)

set(CLI_SRC_FILES
        ${MODEL_SRC_FILES}
        src/view/cli/Cli_base.C)

set(WWW_SRC_FILES
        src/gscd.C
        ${MODEL_SRC_FILES}
        src/Application_controller.C
        src/Navigate.h
        src/web/Abstract_file_view.C
        src/web/Accelerator_button.C
        src/web/Accelerator_text.C
        src/web/Admin_eval_item_widget.C
        src/web/Admin_response_widget.C
        src/web/Admin_view.C
        src/web/Assignments_view.C
        src/web/Base_eval_item_widget.C
        src/web/Confirmation_dialog.C
        src/web/Date_time_edit.C
        src/web/Edit_assignment_view.C
        src/web/Error_view.C
        src/web/Evaluation_view.C
        src/web/Explanation_edit_widget.C
        src/web/Explanation_view_widget.C
        src/web/File_list_widget.C
        src/web/File_manager_view.C
        src/web/File_viewer_widget.C
        src/web/Grading_stats_view.C
        src/web/Grading_view.C
        src/web/Held_back_view.C
        src/web/List_eval_item_widget.C
        src/web/Main_view.C
        src/web/Notification.C
        src/web/Open_am_auth_widget.C
        src/web/Partner_notifications_widget.C
        src/web/Response_widget.C
        src/web/Review_eval_item_widget.C
        src/web/Self_eval_item_widget.C
        src/web/Single_eval_item_widget.C
        src/web/Submission_owner_widget.C
        src/web/Submissions_view.C
        src/web/Unit_line_edit.C
        src/web/User_suggester.C
        src/view/game/HangmanWidget.C
        src/view/game/HighScoresWidget.C
        src/view/game/ImagesWidget.C
        src/view/game/LettersWidget.C
        src/view/game/WordWidget.C)

add_compile_definitions(GSC_AUTH_API_KEY)

find_package(Wt REQUIRED
        COMPONENTS wt dbo dbopostgres
        OPTIONAL_COMPONENTS wthttp wtfcgi)

set(LIBRARIES Wt::Wt Wt::Dbo Wt::DboPostgres)

macro(add_program target)
    add_executable(${ARGV})
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD            17
        CXX_STANDARD_REQUIRED   On
        CXX_EXTENSIONS          Off)
endmacro(add_program)

if(TARGET Wt::HTTP)
    add_program(gscd ${WWW_SRC_FILES})
    target_link_libraries(gscd Wt::HTTP ${LIBRARIES})
    target_compile_definitions(gscd PRIVATE GSC_AUTH_DEBUG)
endif(TARGET Wt::HTTP)

if(TARGET Wt::FCGI)
    add_program(gscd-fcgi ${WWW_SRC_FILES})
    target_link_libraries(gscd-fcgi Wt::FCGI ${LIBRARIES})
    target_compile_definitions(gscd-fcgi PRIVATE GSC_AUTH_OPEN_AM)
endif(TARGET Wt::FCGI)

add_compile_definitions(GSC_AUTH_OPEN_AM)
add_compile_definitions(GSC_AUTH_OPEN_AM)
add_program(gsc-auth src/gsc-auth.C ${CLI_SRC_FILES})
target_link_libraries(gsc-auth ${LIBRARIES})

add_program(gsc-createdb src/gsc-createdb.C ${CLI_SRC_FILES})
target_link_libraries(gsc-createdb ${LIBRARIES})
