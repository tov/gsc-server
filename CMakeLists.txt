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
        src/view/Abstract_file_view.C
        src/view/Accelerator_button.C
        src/view/Accelerator_text.C
        src/view/Admin_eval_item_widget.C
        src/view/Admin_response_widget.C
        src/view/Admin_view.C
        src/view/Assignments_view.C
        src/view/Base_eval_item_widget.C
        src/view/Confirmation_dialog.C
        src/view/Date_time_edit.C
        src/view/Edit_assignment_view.C
        src/view/Error_view.C
        src/view/Evaluation_view.C
        src/view/Explanation_edit_widget.C
        src/view/Explanation_view_widget.C
        src/view/File_list_widget.C
        src/view/File_manager_view.C
        src/view/File_viewer_widget.C
        src/view/Grading_stats_view.C
        src/view/Grading_view.C
        src/view/Held_back_view.C
        src/view/List_eval_item_widget.C
        src/view/Main_view.C
        src/view/Notification.C
        src/view/Open_am_auth_widget.C
        src/view/Partner_notifications_widget.C
        src/view/Response_widget.C
        src/view/Review_eval_item_widget.C
        src/view/Self_eval_item_widget.C
        src/view/Single_eval_item_widget.C
        src/view/Submission_owner_widget.C
        src/view/Submissions_view.C
        src/view/Unit_line_edit.C
        src/view/User_suggester.C
        src/view/game/HangmanWidget.C
        src/view/game/HighScoresWidget.C
        src/view/game/ImagesWidget.C
        src/view/game/LettersWidget.C
        src/view/game/WordWidget.C)

add_definitions(-DGSC_AUTH_OPEN_AM)

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
endif(TARGET Wt::HTTP)

if(TARGET Wt::FCGI)
    add_program(gscd-fcgi ${WWW_SRC_FILES})
    target_link_libraries(gscd-fcgi Wt::FCGI ${LIBRARIES})
endif(TARGET Wt::FCGI)

add_program(gsc-auth src/gsc-auth.C ${CLI_SRC_FILES})
target_link_libraries(gsc-auth ${LIBRARIES})

add_program(gsc-createdb src/gsc-createdb.C ${CLI_SRC_FILES})
target_link_libraries(gsc-createdb ${LIBRARIES})
