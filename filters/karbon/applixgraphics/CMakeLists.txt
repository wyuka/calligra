




########### next target ###############

set(applixgraphicimport_PART_SRCS applixgraphicimport.cc )


kde4_add_plugin(applixgraphicimport ${applixgraphicimport_PART_SRCS})



target_link_libraries(applixgraphicimport  ${KDE4_KDECORE_LIBS} )

install(TARGETS applixgraphicimport  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kontour_applixgraphic_import.desktop  DESTINATION ${SERVICES_INSTALL_DIR})





#original Makefile.am contents follow:

######## General stuff
#
#INCLUDES= -I$(srcdir) $(CALLIGRA_INCLUDES) $(all_includes)
#libapplixgraphicimport_la_LDFLAGS	= -module -avoid-version -no-undefined $(all_libraries)
#
######## Files
#
#kde_module_LTLIBRARIES = libapplixgraphicimport.la
#
#libapplixgraphicimport_la_SOURCES = applixgraphicimport.cc
#libapplixgraphicimport_la_LIBADD = $(CALLIGRA_LIBS)
#noinst_HEADERS = applixgraphicimport.h
#
#METASOURCES = AUTO
#
#service_DATA = kontour_applixgraphic_import.desktop
#servicedir = $(kde_servicesdir)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cc *.cpp -o $(podir)/kontourapplixgraphicsfilter.pot
