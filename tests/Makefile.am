DISTCLEANFILES =
EXTRA_DIST =

TESTS =
check_PROGRAMS =
XFAIL_TESTS =

noinst_LTLIBRARIES = \
	libhud-tests.la

DBUS_RUNNER=dbus-test-runner
XVFB_RUN=". $(srcdir)/run-xvfb.sh"
MOCK_JSON_APP=/usr/lib/indicator-appmenu/mock-json-app

EXTRA_DIST += \
	run-xvfb.sh

#######################
# HUD Test Common Code
#######################

libhud_tests_la_SOURCES = \
	app-list-dummy.h \
	app-list-dummy.c \
	word-list.h \
	hudrandomsource.h \
	hudrandomsource.c \
	hudmanualsource.h \
	hudmanualsource.c \
	hudtestutils.h \
	hudtestutils.c

libhud_tests_la_LDFLAGS = \
	$(top_builddir)/src/libhud-service.la \
	$(DBUSTEST_LIBS) \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

libhud_tests_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-D DBUSMENU_JSON_LOADER=\"$(builddir)/dbusmenu-json-loader\" \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-Wall -Werror

######################
# Test Distance
######################

TESTS += \
	test-distance-test

check_PROGRAMS += \
	test-distance

DISTANCE_XML_REPORT = test-distance.xml

test-distance-test: test-distance Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(DISTANCE_XML_REPORT) $(builddir)/test-distance >> $@
	@echo gtester2xunit $(DISTANCE_XML_REPORT) >> $@
	@chmod +x $@

test_distance_SOURCES = \
	test-distance.c

test_distance_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_distance_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(DISTANCE_XML_REPORT) \
	$(DISTANCE_XML_REPORT:.xml=-xunit.xml)

######################
# Test Hud Item
######################

TESTS += \
	test-hud-item-test

check_PROGRAMS += \
	test-hud-item

HUD_ITEM_XML_REPORT = test-hud-item.xml

test-hud-item-test: test-hud-item Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(HUD_ITEM_XML_REPORT) $(builddir)/test-hud-item >> $@
	@echo gtester2xunit $(HUD_ITEM_XML_REPORT) >> $@
	@chmod +x $@

test_hud_item_SOURCES = \
	test-huditem.c

test_hud_item_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_hud_item_LDADD = \
	$(top_builddir)/src/libhud-service.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(HUD_ITEM_XML_REPORT) \
	$(HUD_ITEM_XML_REPORT:.xml=-xunit.xml)

######################
# Test Result Highlighting
######################

TESTS += \
	test-result-highlighting-test

check_PROGRAMS += \
	test-result-highlighting

RESULT_XML_REPORT = test-result-highlighting.xml

test-result-highlighting-test: test-result-highlighting Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(RESULT_XML_REPORT) $(builddir)/test-result-highlighting >> $@
	@echo gtester2xunit $(RESULT_XML_REPORT) >> $@
	@chmod +x $@

test_result_highlighting_SOURCES = \
	test-result-highlighting.c

test_result_highlighting_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_result_highlighting_LDADD = \
	$(top_builddir)/src/libhud-service.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(RESULT_XML_REPORT) \
	$(RESULT_XML_REPORT:.xml=-xunit.xml)

######################
# Test Menu Input
######################

TESTS += \
	test-menu-input-tester

check_PROGRAMS += \
	dbusmenu-json-loader \
	test-menu-input-model-deep \
	test-menu-input-model-simple \
	test-menu-input-model-shortcuts \
	test-menu-input

MENU_INPUT_XML_REPORT = test-menu-input.xml

test-menu-input-tester: test-menu-input dbusmenu-json-loader test-menu-input-model-simple test-menu-input-model-deep test-menu-input-model-shortcuts Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(MENU_INPUT_XML_REPORT) $(builddir)/test-menu-input >> $@
	@echo gtester2xunit $(MENU_INPUT_XML_REPORT) >> $@
	@chmod +x $@

test_menu_input_SOURCES = \
	test-menu-input.c

test_menu_input_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-D MOCK_JSON_APP=\"$(MOCK_JSON_APP)\" \
	-D MODEL_DEEP=\"$(builddir)/test-menu-input-model-deep\" \
	-D MODEL_SIMPLE=\"$(builddir)/test-menu-input-model-simple\" \
	-D MODEL_SHORTCUTS=\"$(builddir)/test-menu-input-model-shortcuts\" \
	-D JSON_SIMPLE=\"$(srcdir)/test-menu-input-simple.json\" \
	-D JSON_SHORTCUTS=\"$(srcdir)/test-menu-input-shortcuts.json\" \
	-Wall -Werror

test_menu_input_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	libhud-tests.la \
	$(DBUSTEST_LIBS) \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

dbusmenu_json_loader_SOURCES = \
	dbusmenu-json-loader.c

dbusmenu_json_loader_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSMENU_JSON_CFLAGS) \
	-Wall -Werror

dbusmenu_json_loader_LDADD = \
	$(DBUSMENU_JSON_LIBS) \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

test_menu_input_model_deep_SOURCES = \
	test-menu-input-model-deep.c

test_menu_input_model_deep_CFLAGS = \
	$(HUD_CFLAGS) \
	-Wall -Werror

test_menu_input_model_deep_LDADD = \
	$(HUD_LIBS)

test_menu_input_model_simple_SOURCES = \
	test-menu-input-model-simple.c

test_menu_input_model_simple_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-Wall -Werror

test_menu_input_model_simple_LDADD = \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

test_menu_input_model_shortcuts_SOURCES = \
	test-menu-input-model-shortcuts.c

test_menu_input_model_shortcuts_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-Wall -Werror

test_menu_input_model_shortcuts_LDADD = \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	test-menu-input-simple.json \
	test-menu-input-shortcuts.json

DISTCLEANFILES += \
	$(MENU_INPUT_XML_REPORT) \
	$(MENU_INPUT_XML_REPORT:.xml=-xunit.xml)

######################
# Test Keyword Mapping
######################

TESTS += \
	test-keyword-mapping-test

check_PROGRAMS += \
	test-keyword-mapping

KEYWORD_XML_REPORT = test-keyword-mapping.xml

test-keyword-mapping-test: test-keyword-mapping Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo "mkdir -p keyword-mapping/locale/en_FAKELANG/LC_MESSAGES " >> $@
	@echo "$(MSGFMT) --check --verbose --output-file keyword-mapping/locale/en_FAKELANG/LC_MESSAGES/gnome-terminal.mo keyword-mapping/gnome-terminal-en_FAKELANG.po" >> $@
	@echo gtester --verbose -k -o $(KEYWORD_XML_REPORT) $(builddir)/test-keyword-mapping >> $@
	@echo gtester2xunit $(KEYWORD_XML_REPORT) >> $@
	@chmod +x $@

test_keyword_mapping_SOURCES = \
	test-keyword-mapping.c

test_keyword_mapping_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_keyword_mapping_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(KEYWORD_XML_REPORT) \
	$(KEYWORD_XML_REPORT:.xml=-xunit.xml)

######################
# Test String List
######################

TESTS += \
	test-string-list-test

check_PROGRAMS += \
	test-string-list

STRING_LIST_XML_REPORT = test-string-list.xml

test-string-list-test: test-string-list Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(STRING_LIST_XML_REPORT) $(builddir)/test-string-list >> $@
	@echo gtester2xunit $(STRING_LIST_XML_REPORT) >> $@
	@chmod +x $@

test_string_list_SOURCES = \
	test-string-list.c

test_string_list_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_string_list_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(STRING_LIST_XML_REPORT) \
	$(STRING_LIST_XML_REPORT:.xml=-xunit.xml)

######################
# Test Pronounce Dict
######################

TESTS += \
	test-pronounce-dict-test

check_PROGRAMS += \
	test-pronounce-dict

PRONOUNCE_DICT_XML_REPORT = test-pronounce-dict.xml

test-pronounce-dict-test: test-pronounce-dict Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(PRONOUNCE_DICT_XML_REPORT) $(builddir)/test-pronounce-dict >> $@
	@echo gtester2xunit $(PRONOUNCE_DICT_XML_REPORT) >> $@
	@chmod +x $@

test_pronounce_dict_SOURCES = \
	test-pronounce-dict.c

test_pronounce_dict_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_pronounce_dict_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	test-pronounce-dict-hashes.dic \
	test-pronounce-dict-htk.dic \
	test-pronounce-dict-lowercase.dic \
	test-pronounce-dict-semicolon.dic

DISTCLEANFILES += \
	$(PRONOUNCE_DICT_XML_REPORT) \
	$(PRONOUNCE_DICT_XML_REPORT:.xml=-xunit.xml)

######################
# Test Source
######################

TESTS += \
	test-source-test

check_PROGRAMS += \
	test-source

SOURCE_XML_REPORT = test-source.xml

test-source-test: test-source Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(SOURCE_XML_REPORT) $(builddir)/test-source >> $@
	@echo gtester2xunit $(SOURCE_XML_REPORT) >> $@
	@chmod +x $@

test_source_SOURCES = \
	test-source.c

test_source_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-D JSON_INPUT=\"$(srcdir)/test-source.json\" \
	-Wall -Werror

test_source_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(DBUSTEST_LIBS) \
	libhud-tests.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	test-source.json

DISTCLEANFILES += \
	$(SOURCE_XML_REPORT) \
	$(SOURCE_XML_REPORT:.xml=-xunit.xml)

######################
# Test App Indicator Source
######################

TESTS += \
	test-app-indicator-source-test

check_PROGRAMS += \
	test-app-indicator-source

APP_INDICATOR_SOURCE_XML_REPORT = test-app-indicator-source.xml

test-app-indicator-source-test: test-app-indicator-source Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(APP_INDICATOR_SOURCE_XML_REPORT) $(builddir)/test-app-indicator-source >> $@
	@echo gtester2xunit $(APP_INDICATOR_SOURCE_XML_REPORT) >> $@
	@chmod +x $@

test_app_indicator_source_SOURCES = \
	test-app-indicator-source.c

test_app_indicator_source_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-Wall -Werror

test_app_indicator_source_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(DBUSTEST_LIBS) \
	libhud-tests.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	test-app-indicator-source-one.json \
	test-app-indicator-source-two.json \
	test-indicator-source-datetime.json \
	test-indicator-source-messages.json \
	test-indicator-source-session.json \
	test-indicator-source-sound.json \
	test-indicator-source-users.json

DISTCLEANFILES += \
	$(APP_INDICATOR_SOURCE_XML_REPORT) \
	$(APP_INDICATOR_SOURCE_XML_REPORT:.xml=-xunit.xml)

######################
# Test Indicator Source
######################

TESTS += \
	test-indicator-source-test

check_PROGRAMS += \
	test-indicator-source

INDICATOR_SOURCE_XML_REPORT = test-indicator-source.xml

test-indicator-source-test: test-indicator-source Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(INDICATOR_SOURCE_XML_REPORT) $(builddir)/test-indicator-source >> $@
	@echo gtester2xunit $(INDICATOR_SOURCE_XML_REPORT) >> $@
	@chmod +x $@

test_indicator_source_SOURCES = \
	test-indicator-source.c

test_indicator_source_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-Wall -Werror

test_indicator_source_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(DBUSTEST_LIBS) \
	libhud-tests.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(INDICATOR_SOURCE_XML_REPORT) \
	$(INDICATOR_SOURCE_XML_REPORT:.xml=-xunit.xml)

######################
# Test Application List
######################

TESTS += \
	test-application-list-test

check_PROGRAMS += \
	test-application-list

APPLICATION_LIST_XML_REPORT = test-application-list.xml

test-application-list-test: test-application-list Makefile.am
	@echo "#!/bin/bash" > $@
	@echo $(XVFB_RUN) >> $@
	@echo gtester --verbose -k -o $(APPLICATION_LIST_XML_REPORT) $(builddir)/test-application-list >> $@
	@echo gtester2xunit $(APPLICATION_LIST_XML_REPORT) >> $@
	@chmod +x $@

test_application_list_SOURCES = \
	test-application-list.c

test_application_list_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	-Wall -Werror

test_application_list_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(DBUSTEST_LIBS) \
	libhud-tests.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(APPLICATION_LIST_XML_REPORT) \
	$(APPLICATION_LIST_XML_REPORT:.xml=-xunit.xml)

######################
# Test Usage DB Simple
######################

TESTS += \
	test-usage-db-simple-test

check_PROGRAMS += \
	test-usage-db-simple

USAGE_DB_SIMPLE_XML_REPORT = test-usage-db-simple.xml

test-usage-db-simple-test: test-usage-db-simple test-usage-db-simple.sql Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-usage-db-simple.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo cat $(abs_srcdir)/test-usage-db-simple.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo export GSETTINGS_BACKEND=memory >> $@
	@echo export GSETTINGS_SCHEMA_DIR=\`pwd\` >> $@
	@echo gtester --verbose -k -o $(abs_builddir)/$(USAGE_DB_SIMPLE_XML_REPORT) $(abs_builddir)/test-usage-db-simple >> $@
	@echo cd .. >> $@
	@echo rm -rf test-usage-db-simple.dir.\* >> $@
	@echo gtester2xunit $(abs_builddir)/$(USAGE_DB_SIMPLE_XML_REPORT) >> $@
	@chmod +x $@

test_usage_db_simple_SOURCES = \
	test-usage-db-simple.c

test_usage_db_simple_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_usage_db_simple_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(USAGE_DB_SIMPLE_XML_REPORT) \
	$(USAGE_DB_SIMPLE_XML_REPORT:.xml=-xunit.xml)

EXTRA_DIST += test-usage-db-simple.sql

######################
# Test Usage DB Old
######################

TESTS += \
	test-usage-db-old-test

check_PROGRAMS += \
	test-usage-db-old

USAGE_DB_OLD_XML_REPORT = test-usage-db-old.xml

test-usage-db-old-test: test-usage-db-old test-usage-db-old.sql Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-usage-db-old.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo cat $(abs_srcdir)/test-usage-db-old.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo export GSETTINGS_BACKEND=memory >> $@
	@echo export GSETTINGS_SCHEMA_DIR=\`pwd\` >> $@
	@echo gtester --verbose -k -o $(abs_builddir)/$(USAGE_DB_OLD_XML_REPORT) $(abs_builddir)/test-usage-db-old >> $@
	@echo cd .. >> $@
	@echo rm -rf test-usage-db-old.dir.\* >> $@
	@echo gtester2xunit $(abs_builddir)/$(USAGE_DB_OLD_XML_REPORT) >> $@
	@chmod +x $@

test_usage_db_old_SOURCES = \
	test-usage-db-old.c

test_usage_db_old_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_usage_db_old_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

DISTCLEANFILES += \
	$(USAGE_DB_OLD_XML_REPORT) \
	$(USAGE_DB_OLD_XML_REPORT:.xml=-xunit.xml)

EXTRA_DIST += test-usage-db-old.sql

#######################
# Test Usage DB Ancient
#######################

TESTS += \
	test-usage-db-ancient-test

check_PROGRAMS += \
	test-usage-db-ancient

test-usage-db-ancient-test: test-usage-db-ancient test-usage-db-ancient.sql test-usage-dump-entries.sql Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-usage-db-ancient.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo cat $(abs_srcdir)/test-usage-db-ancient.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo export GSETTINGS_BACKEND=memory >> $@
	@echo export GSETTINGS_SCHEMA_DIR=\`pwd\` >> $@
	@echo $(abs_builddir)/test-usage-db-ancient >> $@
	@echo if test \`cat $(abs_srcdir)/test-usage-dump-entries.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite \| wc -l\` -gt 0\; then >> $@
	@echo echo Database has entries >> $@
	@echo exit 1 >> $@
	@echo fi >> $@
	@echo cd .. >> $@
	@echo rm -rf test-usage-db-ancient.dir.\* >> $@
	@chmod +x $@

test_usage_db_ancient_SOURCES = \
	test-usage-db-ancient.c

test_usage_db_ancient_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_usage_db_ancient_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	test-usage-db-ancient.sql \
	test-usage-dump-entries.sql

#######################
# Test Usage DB Testapp
#######################

TESTS += \
	test-usage-db-testapp-test

check_PROGRAMS += \
	test-usage-db-testapp

USAGE_TESTAPP_XML_REPORT = test-usage-db-testapp.xml

test-usage-db-testapp-test: test-usage-db-testapp good-app-info/testapp.desktop.hud-app-info good-app-info/testapp100.desktop.hud-app-info $(top_srcdir)/src/create-db.sql Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-usage-db-testapp.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo cat $(abs_top_srcdir)/src/create-db.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo export HUD_APP_INFO_DIR=$(abs_srcdir)/good-app-info >> $@
	@echo export GSETTINGS_BACKEND=memory >> $@
	@echo export GSETTINGS_SCHEMA_DIR=\`pwd\` >> $@
	@echo gtester --verbose -k -o $(abs_builddir)/$(USAGE_TESTAPP_XML_REPORT) $(abs_builddir)/test-usage-db-testapp >> $@
	@echo cd .. >> $@
	@echo rm -rf test-usage-db-testapp.dir.\* >> $@
	@echo gtester2xunit $(abs_builddir)/$(USAGE_TESTAPP_XML_REPORT) >> $@
	@chmod +x $@

test_usage_db_testapp_SOURCES = \
	test-usage-db-testapp.c

test_usage_db_testapp_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_usage_db_testapp_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

EXTRA_DIST += \
	good-app-info/testapp100.desktop.hud-app-info \
	good-app-info/testapp.desktop.hud-app-info

DISTCLEANFILES += \
	$(USAGE_TESTAPP_XML_REPORT) \
	$(USAGE_TESTAPP_XML_REPORT:.xml=-xunit.xml)

######################
# Test Usage DB Off
######################

TESTS += \
	test-usage-db-off-test

USAGE_OFF_XML_REPORT = test-usage-db-off.xml

test-usage-db-off-test: test-usage-db-testapp good-app-info/testapp.desktop.hud-app-info good-app-info/testapp100.desktop.hud-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-usage-db-off.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo export HUD_APP_INFO_DIR=$(abs_srcdir)/good-app-info >> $@
	@echo export HUD_NO_STORE_USAGE_DATA=1 >> $@
	@echo export GSETTINGS_BACKEND=memory >> $@
	@echo export GSETTINGS_SCHEMA_DIR=\`pwd\` >> $@
	@echo gtester --verbose -k -o $(abs_builddir)/$(USAGE_OFF_XML_REPORT) $(abs_builddir)/test-usage-db-testapp >> $@
	@echo test \! -e indicator-appmenu/hud-usage-log.sqlite >> $@
	@echo cd .. >> $@
	@echo rm -rf test-usage-db-off.dir.\* >> $@
	@echo gtester2xunit $(abs_builddir)/$(USAGE_OFF_XML_REPORT) >> $@
	@chmod +x $@

DISTCLEANFILES += \
	$(USAGE_OFF_XML_REPORT) \
	$(USAGE_OFF_XML_REPORT:.xml=-xunit.xml)

#########################
# Test Bad App Info
#########################

TESTS += \
	test-bad-app-info-dual-header \
	test-bad-app-info-item-no-count \
	test-bad-app-info-item-no-name \
	test-bad-app-info-menu-no-name \
	test-bad-app-info-missing-desktop \
	test-bad-app-info-missing-menus \
	test-bad-app-info-multiple-menus

test-bad-app-info-dual-header: bad-app-info/dual-headers.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/dual-headers.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-item-no-count: bad-app-info/item-no-count.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/item-no-count.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-item-no-name: bad-app-info/item-no-name.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/item-no-name.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-menu-no-name: bad-app-info/menu-no-name.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/menu-no-name.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-missing-menus: bad-app-info/missing-menus.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/missing-menus.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-missing-desktop: bad-app-info/missing-desktop.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/missing-desktop.hud-app-info >> $@
	@chmod +x $@

test-bad-app-info-multiple-menus: bad-app-info/multiple-menus.hud-app-info test-bad-app-info Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo $(builddir)/test-bad-app-info $(srcdir)/bad-app-info/multiple-menus.hud-app-info >> $@
	@chmod +x $@

EXTRA_DIST += \
	bad-app-info/dual-headers.hud-app-info \
	bad-app-info/item-no-count.hud-app-info \
	bad-app-info/item-no-name.hud-app-info \
	bad-app-info/menu-no-name.hud-app-info \
	bad-app-info/missing-desktop.hud-app-info \
	bad-app-info/missing-menus.hud-app-info \
	bad-app-info/multiple-menus.hud-app-info

check_PROGRAMS += \
	test-bad-app-info

test_bad_app_info_SOURCES = \
	test-bad-app-info.c

test_bad_app_info_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_bad_app_info_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

#########################
# Test Good App Info
#########################

TESTS += \
	test-good-app-info-tons-of-entries

check_PROGRAMS += \
	test-load-app-info

test_load_app_info_SOURCES = \
	test-load-app-info.c

test_load_app_info_CFLAGS = \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-Wall -Werror

test_load_app_info_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	$(HUD_LIBS) \
	$(BAMF_LIBS)

test-good-app-info-tons-of-entries: good-app-info/tons-of-entries.hud-app-info test-load-app-info test-usage-dump-entries.sql Makefile.am
	@echo "#!/bin/bash -e" > $@
	@echo cd \`mktemp -d $(abs_builddir)/test-good-app-info-tons-of-entries.dir.XXXXXX\` >> $@
	@echo mkdir -p indicator-appmenu >> $@
	@echo $(abs_builddir)/test-load-app-info indicator-appmenu/hud-usage-log.sqlite $(abs_srcdir)/good-app-info/tons-of-entries.hud-app-info >> $@
	@echo export HUD_CACHE_DIR=\`pwd\` >> $@
	@echo if test \`cat $(abs_srcdir)/test-usage-dump-entries.sql \| sqlite3 indicator-appmenu/hud-usage-log.sqlite \| wc -l\` -ne 540\; then >> $@
	@echo echo Database has wrong number of entries >> $@
	@echo exit 1 >> $@
	@echo fi >> $@
	@echo cd .. >> $@
	@echo rm -rf test-good-app-info-tons-of-entries.dir.\* >> $@
	@chmod +x $@

EXTRA_DIST += \
	good-app-info/tons-of-entries.hud-app-info \
	test-usage-dump-entries.sql

#########################
# Test Dbus Messages
#########################

TESTS += \
	test-dbus-message-count

test-dbus-message-count: test-dbus-message-count.in
	@sed \
		-e "s|\@indicatordir\@|$(INDICATORDIR)|" \
		-e "s|\@top_builddir\@|$(top_builddir)|" \
		-e "s|\@builddir\@|$(builddir)|" \
		-e "s|\@srcdir\@|$(srcdir)|" \
		-e "s|\@mockjsonapp\@|$(MOCK_JSON_APP)|" \
		$< > $@
	@chmod +x $@

EXTRA_DIST += \
	test-dbus-message-count-send-query \
	test-dbus-message-count.in \
	test-dbus-message-count.json

DISTCLEANFILES += test-dbus-message-count.bustle

###################
# Test Performance
###################

check_PROGRAMS += \
	hud-performance

hud_performance_CFLAGS = \
	-I$(top_builddir)/src		\
	-I$(top_srcdir)/src		\
	-I$(top_srcdir)/libhud-client \
	-I$(top_srcdir) \
	-DHUD_CLIENT_COMPILATION \
	$(HUD_CFLAGS) \
	$(BAMF_CFLAGS)
hud_performance_LDADD = \
	$(top_builddir)/src/libhud-service.la		\
	libhud-tests.la \
	$(HUD_LIBS) \
	$(BAMF_LIBS)
hud_performance_SOURCES = \
	hud-performance.c

#####################################
# libhud-client-test
#####################################

TESTS += \
	libhud-client-tester

LIBHUD_XML_REPORT = libhud-client-test.xml
libhud-client-tester: libhud-client-test Makefile.am
	@echo "#!/bin/bash" > $@
	@echo . $(abs_top_srcdir)/tests/run-xvfb.sh >> $@
	@echo gtester --verbose -k -o $(LIBHUD_XML_REPORT) $(abs_builddir)/libhud-client-test >> $@
	@echo gtester2xunit $(LIBHUD_XML_REPORT) >> $@
	@chmod +x $@

DISTCLEANFILES += \
	$(LIBHUD_XML_REPORT) \
	$(LIBHUD_XML_REPORT:.xml=-xunit.xml) \
	libhud-client-tester

libhud_client_test_SOURCES = \
	hud-client-test.c
libhud_client_test_CFLAGS = \
	-D HUD_SERVICE="\"$(top_builddir)/src/hud-service\"" \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	$(HUD_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	$(TEST_CFLAGS)
libhud_client_test_LDADD = \
	$(top_builddir)/libhud-client/libhud-client.la \
	libhud-tests.la \
	$(HUD_LIBS) \
	$(DBUSTEST_LIBS) \
	$(TEST_LIBS)

check_PROGRAMS += libhud-client-test

#########################
# Footer
#########################

DISTCLEANFILES += $(TESTS)
EXTRA_DIST += \
	gobject.suppression
