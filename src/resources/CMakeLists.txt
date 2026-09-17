add_library(taiga-resources STATIC)

target_sources(taiga-resources PUBLIC
	deps.qrc
	gql.qrc
	icons.qrc
	styles.qrc
	sql.qrc
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_sources(taiga PRIVATE
		taiga.manifest
		taiga.rc
	)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	include(GNUInstallDirs)
	install(FILES moe.taiga.Taiga.desktop
		DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
	)
	install(FILES icons/taiga.png
		DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
		RENAME moe.taiga.Taiga.png
	)
endif()

target_link_libraries(taiga-resources
	taiga-config
)
