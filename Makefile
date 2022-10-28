# ----------------------------
#  # Makefile Options
# ----------------------------
#
NAME = C64EMU
ICON = icon.png
DESCRIPTION = "C64 Emulator"
COMPRESSED = NO
ARCHIVED = NO
#
CFLAGS = -Wall -Wextra -Oz
#  CXXFLAGS = -Wall -Wextra
#
#  # ----------------------------

include $(shell cedev-config --makefile)
