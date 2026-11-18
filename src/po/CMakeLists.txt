find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(STATUS "msgfmt not found: translations will not be built")
    return()
endif()

file(GLOB PO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
set(GMO_FILES)
foreach(_po ${PO_FILES})
    get_filename_component(_lang ${_po} NAME_WE)
    set(_gmo ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
    add_custom_command(OUTPUT ${_gmo}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmo} ${_po}
        DEPENDS ${_po} VERBATIM)
    install(FILES ${_gmo} DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${_lang}/LC_MESSAGES RENAME k9copy.mo)
    list(APPEND GMO_FILES ${_gmo})
endforeach()
add_custom_target(translations ALL DEPENDS ${GMO_FILES})
