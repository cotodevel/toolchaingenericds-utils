#
#			Copyright (C) 2017  Coto
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful, but
#WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
#USA
#

#coto: script that builds a collection of objects (*.o) into $(TARGET_LIBRARY_FILE)

#TGDS1.4 compatible Makefile

#Shared
include $(DEFAULT_GCC_PATH)/Makefile.basenewlib
export TARGET_LIBRARY_FILE_OUT	=	libgcc-out.a
export TARGET_LIBRARY_FILE_IN	=	libgcc-in.a

export DIRS_SRC = /
export DIRS_HEADER = /
export BUILD	=	build

# relative path (if you are outside arm9 folder then $(DIR_ARM9), arm7 is $(DIR_ARM7))
RELATIVE_PATH =	

#Prepare ARM9 dirs
DIRLIBS_HDR = $(foreach dirres,$(DIRS_HEADER),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)" )
DIRLIBS_SRC = $(foreach dirres,$(DIRS_SRC),-I "$(CURDIR)/$(RELATIVE_PATH)/$(dirres)" )
file_arm_src_o=  $(foreach dir,$(DIRS_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.o))  )
file_arm_src_txt=  $(foreach dir,$(DIRS_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.txt))  )
file_arm_src_wav=  $(foreach dir,$(DIRS_SRC), $(subst $(CURDIR)/$(RELATIVE_PATH)/$(dir)/,,$(wildcard $(CURDIR)/$(RELATIVE_PATH)/$(dir)/*.wav))  )

# Object Target
objs_arm:=	${file_arm_src_o:.o=.o}

################################################################################################

#Dont modify anything else if not necessary ..
OBJECTS = 	$(objs_arm)
OBJDIR 	=	./$(BUILD)
VPATH 	=	$(DIR_SRC)

# Build Target(s)	(both processors here)
all: usage

$(TARGET_LIBRARY_FILE_OUT): $(OBJECTS)
	-@echo 'Entering Build phase'
	$(AR) rvs	$@	$^
	-@echo 'Leaving Build phase. Output: $@'
	
build:	$(TARGET_LIBRARY_FILE_OUT)

export:	$(TARGET_LIBRARY_FILE_IN)
	-$(AR)	-x	$(TARGET_LIBRARY_FILE_IN)

o2src:
	$(OBJDUMP) -S $(in).o > $(in).txt

%.txt : %.o
	$(OBJDUMP) -S ${@:.txt=.o} > $@

%.ima : %.wav
	$(ADPCMENCODER) -e ${@:.ima=.wav} $@

disassembleall: ${file_arm_src_o:.o=.txt}
	-@echo 'done'

wav2imapcm: ${file_arm_src_wav:.wav=.ima}
	-@echo 'done'

clean:
	-rm	-fr	$(file_arm_src_o)	$(file_arm_src_txt)	$(TARGET_LIBRARY_FILE_OUT)	$(OBJDIR)	${file_arm_src_o:.o=.txt}
usage:
	-@echo 'usage:'
	-@echo 'make build: imports *.o into gnu librarian: $(TARGET_LIBRARY_FILE_OUT)'
	-@echo 'make export: imports gnu librarian $(TARGET_LIBRARY_FILE_IN) into *.o'
	-@echo 'make clean: *.o cleanup'
	-@echo 'make obj disassemble: make in=filename o2src (filename without extension)'
	-@echo 'Disassemble *.o into txt: make export -> make disassembleall'
	-@echo 'make wav2imapcm: Creates and performs IMA-ADPCM (Intel) compression from uncompressed *.WAV files in this directory. '