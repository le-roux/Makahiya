##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT =
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT =
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = ch

# Imported source files and paths
CHIBIOS = ../ChibiOS_16.1.5
# Startup files.
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/startup_stm32f4xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include board/board.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk

# Define linker script file here
LDSCRIPT= ./STM32F405xG.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
SRC := ./src
CSRC = $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(TESTSRC) \
       $(LWSRC) \
       $(FATFSSRC) \
       $(CHIBIOS)/os/various/evtimer.c \
       $(CHIBIOS)/os/hal/lib/streams/memstreams.c \
       $(CHIBIOS)/os/hal/lib/streams/chprintf.c \
       $(CHIBIOS)/os/various/shell.c \
       $(SRC)/main.c \
       $(SRC)/pwmdriver.c \
       $(SRC)/bluetooth.c \
       $(SRC)/RTT_streams.c \
       $(SRC)/../SEGGER_RTT/SEGGER_RTT.c \
       $(SRC)/../SEGGER_RTT/SEGGER_RTT_printf.c \
       $(SRC)/i2c_user.c \
       $(SRC)/fdc2214.c \
       $(SRC)/ext_user.c \
       $(SRC)/capacitive_sensor.c \
	   $(SRC)/alarm.c \
	   $(SRC)/flash.c \
	   $(SRC)/serial_user.c \
	   $(SRC)/sound.c \
	   $(SRC)/utils.c \
	   $(SRC)/websocket.c \
	   $(SRC)/wifi.c \
	   $(SRC)/i2s_user.c \
	   $(SRC)/commands.c

# MP3 files to include in the executable.
MP3SRC = alarm1.mp3 alarm2.mp3 alarm3.mp3 boot.mp3

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(LWINC) $(FATFSINC) \
         $(CHIBIOS)/os/hal/lib/streams $(CHIBIOS)/os/various \
	 $(CHIBIOS)/../Project/includes \
	 $(CHIBIOS)/../Project/SEGGER_RTT

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes -Wno-strict-aliasing -std=gnu99

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = -lm -lhelix -L ../helix

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk

$(BUILDDIR)/obj/%.o: %.mp3
	$(TRGT)objcopy --rename-section .data=.rodata,alloc,load,readonly,data,contents -I binary -O elf32-littlearm -B arm $^ $@

../helix/libhelix.a:
	make -C ../helix

$(BUILDDIR)/ch.elf: ../helix/libhelix.a

jlink:
	JLinkGDBServer -if swd -device "STM32F405VG" -LocalHostOnly

gdb: $(BUILDDIR)/ch.elf
	$(TRGT)gdb $(BUILDDIR)/ch.elf

TEST_DIR := ./test_dir
INCLUDES := ./includes/
test: $(TEST_DIR)/test.c $(TEST_DIR)/utils.o $(TEST_DIR)/wifi.o
	gcc -c $(TEST_DIR)/test.c -o $(TEST_DIR)/test.o -iquote $(INCLUDES)
	gcc $(TEST_DIR)/test.o $(TEST_DIR)/utils.o $(TEST_DIR)/wifi.o -lcriterion -o $(TEST_DIR)/$@

$(TEST_DIR)/%.o: $(SRC)/%.c
	gcc -c $^ -o $@ -iquote $(INCLUDES) -std=c99 -DTEST

run_test: test
	$(TEST_DIR)/test

clean_test:
	rm -f $(TEST_DIR)/*.o
	rm -f $(TEST_DIR)/test
