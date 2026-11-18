# Private action icons, looked up at runtime from ${K9_DATADIR}/icons
file(GLOB K9_ICONS ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
install(FILES ${K9_ICONS} DESTINATION ${CMAKE_INSTALL_DATADIR}/k9copy/icons)
