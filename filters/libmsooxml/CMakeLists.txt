#add_definitions(-DKDE_DEFAULT_DEBUG_AREA=30529)

if(QCA2_FOUND)
    add_definitions( -DHAVE_QCA2 )
    include_directories(${QCA2_INCLUDE_DIR})
endif(QCA2_FOUND)

include_directories(
    ${KOMAIN_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIRECTORY}
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx # for Charting
)

########### next target ###############

set(msooxml_LIB_SRCS
    MsooXmlGlobal.cpp
    MsooXmlUtils.cpp
    MsooXmlReader.cpp
    MsooXmlCommonReader.cpp
    MsooXmlThemesReader.cpp
    MsooXmlRelationshipsReader.cpp
    MsooXmlRelationships.cpp
    MsooXmlImport.cpp
    MsooXmlDocPropertiesReader.cpp
    MsooXmlDiagramReader.cpp
    MsooXmlDiagramReader_p.cpp
    ooxml_pole.cpp
    MsooXmlDrawingTableStyleReader.cpp
    MsooXmlDrawingTableStyle.cpp
    MsooXmlTableStyle.cpp
    VmlDrawingReader.cpp
    PredefinedShapeHelper.cpp
    ComplexShapeHandler.cpp
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx/ChartExport.cpp
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx/NumberFormatParser.cpp
    # TODO: move back to kolibs
    KoOdfExporter.cpp
)

kde4_add_library(msooxml SHARED ${msooxml_LIB_SRCS})

target_link_libraries(msooxml koodf komain)
if(QCA2_FOUND)
   target_link_libraries(msooxml ${QCA2_LIBRARIES})
endif(QCA2_FOUND)

set_target_properties(msooxml PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )
install(TARGETS msooxml ${INSTALL_TARGETS_DEFAULT_ARGS})

find_package(SharedMimeInfo)
macro_log_feature(SHARED_MIME_INFO_FOUND "SharedMimeInfo" "Shared Mime Info" "http://freedesktop.org/wiki/Software/shared-mime-info" TRUE "" "Required to determine MSOOXML file types.")

install(FILES msooxml-all.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

if (FALSE) # these headers are private for now
install( FILES
    msooxml_export.h
    MsooXmlContentTypes.h
    MsooXmlReader.h
    MsooXmlReader_p.h
    MsooXmlCommonReaderImpl.h
    MsooXmlCommonReader.h
    MsooXmlCommonReaderMethods.h
    MsooXmlSchemas.h
    MsooXmlThemesReader.h
    MsooXmlUtils.h
    MsooXmlRelationships.h
    MsooXmlImport.h
    MsooXmlCommentsReader.h
    MsooXmlDocPropertiesReader.h
    MsooXmlNotesReader.h
    MsooXmlDiagramReader.h
    MsooXmlDiagramReader_p.h
    MsooXmlDrawingTableStyleReader.h
    MsooXmlDrawingTableStyle.h
    MsooXmlDrawingMLShared.h
    MsooXmlDrawingMLSharedImpl.h
    VmlDrawingReader.h
    ComplexShapeHandler.h
    PredefinedShapeHelper.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/msooxml COMPONENT Devel
)
endif (FALSE)
