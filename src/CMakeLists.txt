cmake_minimum_required(VERSION 3.5)
project(gsc-src CXX)

add_compile_definitions(GSC_AUTH_API_KEY)
add_compile_definitions(GSC_SHOW_QUOTA)

set(MODEL_SRC_FILES
        common/env_var.C
        common/format.C
        common/paths.C
        common/Media_type_registry.C
        Session.C
        model/specializations.h
        model/auth/Api_key.C
        model/auth/Auth_token.C
        model/auth/Environment.C
        model/auth/User.C
        model/game/Dictionary.C
        model/game/User_stats.C
        model/Abstract_evaluation.C
        model/Assignment.C
        model/Eval_item.C
        model/Exam_grade.C
        model/File_data.C
        model/File_meta.C
        model/Grader_eval.C
        model/Partner_request.C
        model/Permalink.C
        model/Self_eval.C
        model/Submission.C)

set(CLI_SRC_FILES
        ${MODEL_SRC_FILES}
        view/cli/Cli_base.C)

set(WWW_SRC_FILES
        gscd.C
        ${MODEL_SRC_FILES}
        Application_controller.C
        Navigate.h
        view/api/Endpoint.C
        view/api/Http_status.C
        view/api/Request_body.C
        view/api/Request_handler.C
        view/api/Result_array.C
        view/api/resources.C
        view/web/Confirmation_dialog.C
        view/web/Notification.C
        view/web/Submission_context.C
        view/web/view/Abstract_file_view.C
        view/web/view/Admin_view.C
        view/web/view/Assignments_view.C
        view/web/view/Edit_assignment_view.C
        view/web/view/Error_view.C
        view/web/view/Evaluation_view.C
        view/web/view/File_manager_view.C
        view/web/view/Grading_stats_view.C
        view/web/view/Grading_view.C
        view/web/view/Held_back_view.C
        view/web/view/Main_view.C
        view/web/view/Profile_view.C
        view/web/view/Submissions_view.C
        view/web/widget/Accelerator_button.C
        view/web/widget/Accelerator_text.C
        view/web/widget/Admin_eval_item_widget.C
        view/web/widget/Admin_response_widget.C
        view/web/widget/Api_key_widget.C
        view/web/widget/Base_eval_item_widget.C
        view/web/widget/Date_time_edit.C
        view/web/widget/Explanation_edit_widget.C
        view/web/widget/Explanation_view_widget.C
        view/web/widget/File_list_widget.C
        view/web/widget/File_viewer_widget.C
        view/web/widget/List_eval_item_widget.C
        view/web/widget/Open_am_auth_widget.C
        view/web/widget/Partner_notification_widget.C
        view/web/widget/Response_widget.C
        view/web/widget/Review_eval_item_widget.C
        view/web/widget/Self_eval_item_widget.C
        view/web/widget/Single_eval_item_widget.C
        view/web/widget/Submission_owner_widget.C
        view/web/widget/Unit_line_edit.C
        view/web/widget/User_suggester.C
        view/game/HangmanWidget.C
        view/game/HighScoresWidget.C
        view/game/ImagesWidget.C
        view/game/LettersWidget.C
        view/game/WordWidget.C)

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

#add_custom_command(
#        OUTPUT server_root/html/css/gsc.css
#        COMMAND sass gsc.scss gsc.css
#        MAIN_DEPENDENCY server_root/html/css/gsc.scss
#        WORKING_DIRECTORY server_root/html/css/
#)

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

add_program(gsc-auth gsc-auth.C ${CLI_SRC_FILES})
target_link_libraries(gsc-auth ${LIBRARIES})

add_program(gsc-createdb gsc-createdb.C ${CLI_SRC_FILES})
target_link_libraries(gsc-createdb ${LIBRARIES})
