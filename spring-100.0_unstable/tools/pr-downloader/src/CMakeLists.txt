subdirs(lib)

if (PRD_ARCHIVE_SUPPORT)
	set(archivessrc FileSystem/SevenZipArchive.cpp FileSystem/ZipArchive.cpp)
	set(archiveslib pr-7z)
	add_definitions(-DARCHIVE_SUPPORT)
endif()

add_library(Downloader STATIC
	Downloader/Rapid/RapidDownloader.cpp
	Downloader/Rapid/Repo.cpp
	Downloader/Rapid/Sdp.cpp
	Downloader/Http/HttpDownloader.cpp
	Downloader/Http/DownloadData.cpp
#	Downloader/Plasma/PlasmaDownloader.cpp
	Downloader/CurlWrapper.cpp
	Downloader/Download.cpp
	Downloader/IDownloader.cpp
	Downloader/Mirror.cpp
	Downloader/IDownloadsObserver.cpp
	FileSystem/AtomicFile.cpp
	FileSystem/FileData.cpp
	FileSystem/FileSystem.cpp
	FileSystem/File.cpp
	FileSystem/HashMD5.cpp
	FileSystem/HashSHA1.cpp
	FileSystem/HashCRC32.cpp
	FileSystem/IHash.cpp
	Util.cpp
	Logger.cpp
	Version.cpp
	${archivessrc}
)

set_source_files_properties(Version.cpp PROPERTIES COMPILE_DEFINITIONS "PR_DOWNLOADER_VERSION=${PR_DOWNLOADER_VERSION}")

IF (WIN32)
	SET(WIN32LIBS ws2_32 mswsock)
ENDIF ()

set(PRDOWNLOADER_LIBS
	${WIN32LIBS}
	Downloader
)

target_link_libraries(Downloader
	${CURL_LIBRARIES}
	${OPENSSL_LIBRARIES}
	xmlrpc
	${WIN32LIBS}
#	soap
	${ZLIB_LIBRARIES}
	pr-md5
	pr-sha1
	bencode
	${archiveslib}
)

if(PRD_ARCHIVE_SUPPORT)
	if (MINIZIP_FOUND)
		target_link_libraries(Downloader ${MINIZIP_LIBRARIES})
	else()
		target_link_libraries(Downloader pr-minizip)
	endif()
endif()


set (PRDOWNLOADER "pr-downloader")
set (PRDOWNLOADER_SHARED ${PRDOWNLOADER}_shared)
set (PRDOWNLOADER_STATIC ${PRDOWNLOADER}_static)






OPTION(PRD_SHAREDLIB
	"Enables compilation of the shared lib" ON)

if (PRD_SHAREDLIB)
	add_library(${PRDOWNLOADER_SHARED} SHARED
		pr-downloader.cpp
	)
	if(PRD_DO_INSTALL AND (PRD_DEVELOP_FILES OR (PRD_CONSOLETOOL AND NOT PRD_LINK_STATIC)))
		INSTALL (TARGETS ${PRDOWNLOADER_SHARED}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_SHARED} ${PRDOWNLOADER_LIBS} )
endif()

OPTION(PRD_STATICLIB
	"Enables compilation of the static lib" ON)
if (PRD_STATICLIB)
	add_library(${PRDOWNLOADER_STATIC} STATIC
		pr-downloader.cpp
	)
	if(PRD_DO_INSTALL AND PRD_DEVELOP_FILES)
		INSTALL (TARGETS ${PRDOWNLOADER_STATIC}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
	target_link_libraries( ${PRDOWNLOADER_STATIC} ${PRDOWNLOADER_LIBS} ${CMAKE_DL_LIBS})
endif()

OPTION(PRD_CONSOLETOOL
        "Enables compilation and installation of the console tool ${PRDOWNLOADER}" ON)

if (PRD_CONSOLETOOL)
	add_executable(${PRDOWNLOADER}
		main.cpp
	)

	if (PRD_LINK_STATIC)
		target_link_libraries( ${PRDOWNLOADER} ${PRDOWNLOADER_STATIC} )
	else  ()
		# hack arround "make install" doesn't generate libpr-downloader_shared.so
		add_dependencies(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED})
		target_link_libraries(${PRDOWNLOADER} ${PRDOWNLOADER_SHARED} )
	endif ()
	if (WIN32)
		set_target_properties(${PRDOWNLOADER} PROPERTIES LINK_FLAGS "-Wl,-subsystem,console")
	endif()
	if (PRD_DO_INSTALL)
		INSTALL (TARGETS ${PRDOWNLOADER}
			RUNTIME DESTINATION ${PRD_BINDIR}
			LIBRARY DESTINATION ${PRD_LIBDIR}
			ARCHIVE DESTINATION ${PRD_LIBDIR} )
	endif()
endif(PRD_CONSOLETOOL)

FILE( GLOB_RECURSE header "*.h" )



if(NOT WIN32 AND PRD_DEVELOP_FILES)
	SET(VERSION ${PR_DOWNLOADER_VERSION})
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libspringdownloader.pc.in"
	               "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" @ONLY)
	if (PRD_DO_INSTALL)
		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libspringdownloader.pc" DESTINATION "${PRD_LIBDIR}/pkgconfig")
	endif()
endif()

if(PRD_DO_INSTALL AND PRD_DEVELOP_FILES)
	INSTALL (FILES
		pr-downloader.h
	        DESTINATION ${PRD_INCLUDE_DIR}/Downloader COMPONENT Devel )
endif()

