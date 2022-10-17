# ----------------------------
#  # Makefile Options
# ----------------------------
#
NAME = C64EMU
ICON = icon.png
DESCRIPTION = "C64 Emulator"
COMPRESSED = YES
ARCHIVED = NO
#
CFLAGS = -Wall -Wextra -Oz
#  CXXFLAGS = -Wall -Wextra
#
#  # ----------------------------

include $(shell cedev-config --makefile)
