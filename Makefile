# ##############################################################################
#
#              """          Makefile
#       -\-    _|__
#        |\___/  . \        Created on 23 Apr. 2024 at 18:22
#        \     /(((/        by hmelica
#         \___/)))/         hmelica@student.42.fr
#
# ##############################################################################

###################
##  PRINT COLOR  ##
###################

RESET		= \033[0m
BLACK		= \033[0;30m
RED			= \033[0;31m
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
MAGENTA		= \033[0;35m
CYAN		= \033[0;36m
GRAY		= \033[0;37m
DELETE		= \033[2K\r
GO_UP		= \033[1A\r

###################
##  PROJECT VAR  ##
###################

PROJECT=bruteforce
PICO_BOARD=pico

########################
##  AUTO-EDIT ON VAR  ##
########################

DEBUG=${if ${filter ${MAKECMDGOALS}, debug}, -DCMAKE_BUILD_TYPE=Debug, }
WEB_HEADERS=${patsubst %,%.h,$(WEB_SRCS)}

all: build ${WEB_HEADERS}
	cd build && cmake .. -DPICO_BOARD=${PICO_BOARD} ${DEBUG} && make -j8

force:;

push:
	cd build && openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program $(PROJECT).elf verify reset"

listen:
	minicom -b 115200 -o -D /dev/tty.usbmodem146102

gdb:
	gdb ./build/$(PROJECT).elf

build:
	mkdir build

fclean:
	rm -rf build

re: fclean all

debug: all
	@printf "${MAGENTA}Debug mode${RESET}\n"
