# Cmake options
#
# The following options can be set on the command line by specifying the GNU make variable name.
# Extra options can also be provided to cmake by setting EXTRA_CMAKE_ARGS. The options,
# descriptions, and default values are listed below.

EXTRA_CMAKE_ARGS ?=

# The directory for cmake to place build files.
BUILD_DIR ?= build

# The build type for cmake to use.
CMAKE_BUILD_TYPE ?= Release


.PHONY: all configure build clean


all: configure build


configure:
	cmake -B $(BUILD_DIR)                           \
		-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)  \
		$(EXTRA_CMAKE_ARGS)


build:
	cmake --build $(BUILD_DIR)


clean:
	rm -rf $(BUILD_DIR)
