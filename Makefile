#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := MEX-10_WIFI_ESP32

COMPONENT_ADD_INCLUDEDIRS := components/speech_wakeup
COMPONENT_ADD_INCLUDEDIRS := components/music
COMPONENT_ADD_INCLUDEDIRS := components/test
include $(IDF_PATH)/make/project.mk

