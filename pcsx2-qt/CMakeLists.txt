set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(pcsx2-qt)
set_target_properties(pcsx2-qt PROPERTIES OUTPUT_NAME "aethersx2")

if (PACKAGE_MODE)
	install(TARGETS pcsx2-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
	install(TARGETS pcsx2-qt DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()

# Copy resource files if needed
if(APPLE)
	function(pcsx2_resource path basedir)
	        get_filename_component(dir ${path} DIRECTORY)
	        file(RELATIVE_PATH subdir ${basedir} ${dir})
                target_sources(pcsx2-qt PRIVATE ${path})
                set_source_files_properties(${path} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${subdir})
	endfunction()
endif()

target_sources(pcsx2-qt PRIVATE
	AboutDialog.cpp
	AboutDialog.h
	AboutDialog.ui
	DisplayWidget.cpp
	DisplayWidget.h
	EmuThread.cpp
	EmuThread.h
	Main.cpp
	MainWindow.cpp
	MainWindow.h
	MainWindow.ui
	PrecompiledHeader.cpp
	PrecompiledHeader.h
	QtHost.cpp
	QtHost.h
	QtKeyCodes.cpp
	QtUtils.cpp
	QtUtils.h
	SettingWidgetBinder.h
	GameList/GameListModel.cpp
	GameList/GameListModel.h
	GameList/GameListRefreshThread.cpp
	GameList/GameListRefreshThread.h
	GameList/GameListWidget.cpp
	GameList/GameListWidget.h
	Settings/AdvancedSystemSettingsWidget.cpp
	Settings/AdvancedSystemSettingsWidget.h
	Settings/AdvancedSystemSettingsWidget.ui
	Settings/AudioSettingsWidget.cpp
	Settings/AudioSettingsWidget.h
	Settings/AudioSettingsWidget.ui
	Settings/BIOSSettingsWidget.cpp
	Settings/BIOSSettingsWidget.h
	Settings/BIOSSettingsWidget.ui
	Settings/ControllerBindingWidget.ui
	Settings/ControllerBindingWidget_DualShock2.ui
	Settings/ControllerBindingWidgets.cpp
	Settings/ControllerBindingWidgets.h
	Settings/ControllerGlobalSettingsWidget.cpp
	Settings/ControllerGlobalSettingsWidget.h
	Settings/ControllerGlobalSettingsWidget.ui
	Settings/ControllerSettingsDialog.cpp
	Settings/ControllerSettingsDialog.h
	Settings/ControllerSettingsDialog.ui
	Settings/CreateMemoryCardDialog.cpp
	Settings/CreateMemoryCardDialog.h
	Settings/CreateMemoryCardDialog.ui
	Settings/EmulationSettingsWidget.cpp
	Settings/EmulationSettingsWidget.h
	Settings/EmulationSettingsWidget.ui
	Settings/GameFixSettingsWidget.cpp
	Settings/GameFixSettingsWidget.h
	Settings/GameFixSettingsWidget.ui
	Settings/GameListSettingsWidget.cpp
	Settings/GameListSettingsWidget.h
	Settings/GameListSettingsWidget.ui
	Settings/GameSummaryWidget.cpp
	Settings/GameSummaryWidget.h
	Settings/GameSummaryWidget.ui
	Settings/GraphicsSettingsWidget.cpp
	Settings/GraphicsSettingsWidget.h
	Settings/GraphicsSettingsWidget.ui
	Settings/HotkeySettingsWidget.cpp
	Settings/HotkeySettingsWidget.h
	Settings/InputBindingDialog.cpp
	Settings/InputBindingDialog.h
	Settings/InputBindingDialog.ui
	Settings/InputBindingWidget.cpp
	Settings/InputBindingWidget.h
	Settings/InterfaceSettingsWidget.cpp
	Settings/InterfaceSettingsWidget.h
	Settings/InterfaceSettingsWidget.ui
	Settings/MemoryCardSettingsWidget.cpp
	Settings/MemoryCardSettingsWidget.h
	Settings/MemoryCardSettingsWidget.ui
	Settings/SettingsDialog.cpp
	Settings/SettingsDialog.h
	Settings/SettingsDialog.ui
	Settings/SystemSettingsWidget.cpp
	Settings/SystemSettingsWidget.h
	Settings/SystemSettingsWidget.ui
	resources/resources.qrc
)

target_precompile_headers(pcsx2-qt PRIVATE PrecompiledHeader.h)

target_include_directories(pcsx2-qt PRIVATE
	${Qt6Gui_PRIVATE_INCLUDE_DIRS}
	"${CMAKE_BINARY_DIR}/common/include"
	"${CMAKE_SOURCE_DIR}/pcsx2"
	"${CMAKE_SOURCE_DIR}/pcsx2-qt"
)

target_link_libraries(pcsx2-qt PRIVATE
	PCSX2_FLAGS
	PCSX2
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Network
)

if (APPLE)
	set_target_properties(pcsx2-qt PROPERTIES
		MACOSX_BUNDLE true
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
		OUTPUT_NAME AetherSX2
	)

	file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_SOURCE_DIR}/bin/resources/*)
	foreach(path IN LISTS RESOURCE_FILES)
	        get_filename_component(file ${path} NAME)
	        if("${file}" MATCHES "^\\.") # Don't copy macOS garbage (mainly Finder's .DS_Store files) into application
	                continue()
	        endif()
	        if (NOT WIN32 AND "${path}" MATCHES "/dx11/") # Don't include unneccessary stuff
	                continue()
	        endif()
	        if ("${path}" MATCHES "/locale/") # Generate locales with gettext instead of copying them from bin
	                continue()
	        endif()
	        pcsx2_resource(${path} ${CMAKE_SOURCE_DIR}/bin/resources/)
		message("res ${path}")
	endforeach()

	# Import metallib files.
	add_custom_command(TARGET pcsx2-qt POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE_DIR:PCSX2>/*.metallib "$<TARGET_FILE_DIR:pcsx2-qt>/../Resources/"
	)

	# Icon file.
	pcsx2_resource(${CMAKE_CURRENT_SOURCE_DIR}/AetherSX2.icns ${CMAKE_CURRENT_SOURCE_DIR})

	# If they say to skip postprocess bundle, leave the target in but make it so they have
	# to manually run it
	if (SKIP_POSTPROCESS_BUNDLE)
		set(postprocessBundleType "")
	else()
		set(postprocessBundleType ALL)

		#add_custom_target(pcsx2-postprocess-bundle ${postprocessBundleType}
		#	COMMAND ${CMAKE_COMMAND} "-DPCSX2_BUNDLE_PATH=$<TARGET_FILE_DIR:pcsx2-qt>/../.."
		#		-P ${CMAKE_SOURCE_DIR}/cmake/Pcsx2PostprocessBundle.cmake
		#)
		#add_dependencies(pcsx2-postprocess-bundle pcsx2-qt)

		# Call macdeployqt to add the plugins into AetherSX2.app
		get_target_property(MOC_EXECUTABLE_LOCATION Qt6::moc IMPORTED_LOCATION)
		get_filename_component(QT_BINARY_DIRECTORY "${MOC_EXECUTABLE_LOCATION}" DIRECTORY)
		find_program(MACDEPLOYQT_EXE macdeployqt HINTS "${QT_BINARY_DIRECTORY}")
		add_custom_command(TARGET pcsx2-qt POST_BUILD
			COMMAND "${MACDEPLOYQT_EXE}"
			"$<TARGET_FILE_DIR:pcsx2-qt>/../.."
		)
	endif()
endif()

