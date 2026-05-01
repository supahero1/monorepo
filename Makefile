#   Copyright 2024-2026 Franciszek Balcerak
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

RM := rm -f
CP := cp
MV := mv
SHELL := bash

ifeq ($(OS),Windows_NT)
LOCALE := LC_ALL=C LANG=C
else
LOCALE :=
endif


ifeq ($(VALGRIND),1)
VALGRIND_CALL := valgrind --leak-check=full --show-leak-kinds=all \
	--suppressions=../val_sup.txt --log-file="val_log.txt" --
endif

ifeq ($(KCACHEGRIND),1)
VALGRIND_CALL := valgrind --tool=callgrind --
endif


TEX_DIRS := $(wildcard tex/img/[0-9]*/)
TEX_FILES := $(TEX_DIRS:tex/img/%/=tex/dds/%.dds)

SRC_TEX_DIRS := src/client/tex/ include/client/tex/
SRC_FONT_DIRS := src/client/font/ include/client/font/


.PHONY: all
all:
	@printf "Specify one (or more) of the following:\n\
	\n\
	font_build  generates font textures and sources\n\
	font_gen    generates only font sources (fast, for dev)\n\
	font_clean  removes font textures\n\
	font_wipe   above + sources\n\
	\n\
	var_build   generates variable textures\n\
	var_clean   removes variable textures\n\
	\n\
	tex_build   combines all textures\n\
	tex_gen     generates texture sources\n\
	tex_clean   removes all combined textures\n\
	tex_wipe    above + sources\n\
	\n\
	dds_build   packs and compresses combined textures\n\
	dds_clean   removes raw dds files\n\
	dds_wipe    above + compressed dds files\n\
	\n\
	tex_reset   does all of the above to fulfill client prerequisites\n\
	\n\
	client      generates the client, requires dds and all sources\n\
	server      generates the server, no prerequisites\n\
	test        runs any modified tests\n\
	cloc        = everything - libraries - auto generated stuff\n\
	\n\
	clean       removes any built executables\n\
	wipe        above + all textures + all sources (= everything)\n\
	\n\
	You can use the clean targets to remove intermediate files.\n\
	Valid use case:\n\
		make font_build tex_build font_clean   # <- tex_build uses font\n\
	Invalid use case:\n\
		make font_build font_clean             # <- destroyed without using it\n\
	Not all commands are compatible with each other in a chain like above.\n\
	Unless you deeply know how they work, use one command per a \`make\` call.\n\
	\n\
	Specify RELEASE=1 for a production build\n\
	Specify RELEASE=2 for a native build (faster than production but not portable)\n"


tex/font/ tex/var/ tex/img/ tex/dds_raw/ tex/dds_bc1/ tex/dds/:
	mkdir -p $@

bin/shaders/ $(SRC_TEX_DIRS) $(SRC_FONT_DIRS) release/shaders/:
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r bin/

.PHONY: wipe
wipe: clean font_wipe var_clean tex_wipe dds_wipe


FONT_GEN_EXE := bin/tex/font_gen

.PHONY: font_build
font_build: $(FONT_GEN_EXE) | tex/font/ $(SRC_FONT_DIRS)
	$(RM) -r tex/font/*
	WRITE_IMG=1 ./$(FONT_GEN_EXE)

.PHONY: font_gen
font_gen: $(FONT_GEN_EXE) | $(SRC_FONT_DIRS)
	./$(FONT_GEN_EXE)

.PHONY: font_clean
font_clean:
	$(RM) -r tex/font/

.PHONY: font_wipe
font_wipe: font_clean
	$(RM) -r $(SRC_FONT_DIRS)


VAR_GEN_EXE := bin/tex/var_gen

.PHONY: var_build
var_build: $(VAR_GEN_EXE) | tex/var/
	$(RM) -r tex/var/*
	./$(VAR_GEN_EXE)

.PHONY: var_clean
var_clean:
	$(RM) -r tex/var/


SORT_EXE := bin/tex/sort
TEX_GEN_EXE := bin/tex/tex_gen

.PHONY: tex_build
tex_build: $(SORT_EXE) | tex/img/
	$(RM) -r tex/img/*
	$(CP) -r tex/const/* tex/img/
	$(CP) -r tex/font/* tex/img/
	$(CP) -r tex/var/* tex/img/
	./$(SORT_EXE)

.PHONY: tex_gen
tex_gen: $(TEX_GEN_EXE) | tex/img/ $(SRC_TEX_DIRS)
	./$(TEX_GEN_EXE)
	TEX_COUNT=$$(grep -oP '#define TEX__COUNT \K\d+' include/client/tex/base.h); \
	sed -i "s/inTex\[[0-9]*\];/inTex[$$TEX_COUNT];/g" shaders/frag.glsl

.PHONY: tex_clean
tex_clean:
	$(RM) -r tex/img/

.PHONY: tex_wipe
tex_wipe: tex_clean
	$(RM) -r $(SRC_TEX_DIRS)


.PRECIOUS: tex/dds_raw/%.dds
tex/dds_raw/%.dds: tex/img/% | tex/dds_raw/
	pngs=$$(ls -1 "$<" | wc -l); \
	if [ "$$pngs" -eq 1 ]; then \
		$(CP) $</* $(shell dirname $@)/$*.png; \
		$(LOCALE) texconv -w $* -h $* -m 1 -f BGRA -o \
			$(shell dirname $@) -l -y -- $(shell dirname $@)/$*.png; \
	else \
		$(LOCALE) texassemble array -w $* -h $* -f BGRA -o $@ -l -y -stripmips -- $</*.png; \
	fi

.PRECIOUS: tex/dds_bc1/%.dds
tex/dds_bc1/%.dds: tex/dds_raw/%.dds | tex/dds_bc1/
	$(LOCALE) texconv -w $* -h $* -m 1 -f BC1_UNORM_SRGB -o $(shell dirname $@) -l -y -- $<

.PRECIOUS: tex/dds/%.dds
tex/dds/%.dds: tex/dds_bc1/%.dds | tex/dds/
	-zstd -T0 --ultra -20 -o $@ $<

.PHONY: dds_build
dds_build: $(TEX_FILES)

.PHONY: dds_clean
dds_clean:
	$(RM) -r tex/dds_raw/ tex/dds_bc1/

.PHONY: dds_wipe
dds_wipe: dds_clean
	$(RM) -r tex/dds/


.PHONY: tex_reset
tex_reset:
	$(MAKE) wipe
	RELEASE=0 scons tex/font_gen tex/var_gen tex/tex_gen tex/sort -j $(shell nproc)
	RELEASE=0 $(MAKE) font_build var_build tex_build tex_gen
	$(MAKE) dds_build


bin/shaders/%.spv: shaders/%.glsl | bin/shaders/ release/shaders/
	glslc -O -fshader-stage=$* $< -o $@
	$(CP) $@ release/shaders/

.PHONY: shaders
shaders: bin/shaders/vert.spv bin/shaders/frag.spv


bin/tex/%: | bin/tex/
	scons tex/$* -j $(shell nproc)


.PHONY: client
client: shaders
	if [[ ! -f include/client/tex/base.h || ! -d tex/dds/ ]]; then \
		echo "Run \`make tex_reset\` first."; \
		exit 1; \
	fi

	scons client -j $(shell nproc)

	if [[ ! -f release/Ubuntu.ttf ]]; then \
		$(CP) tex/Ubuntu.ttf release/; \
	fi

	if [[ ! -d release/shaders/ ]]; then \
		$(CP) -r bin/shaders/ release/; \
	fi

	$(CP) bin/client release/

	mkdir -p release/textures/
	$(CP) tex/dds/* release/textures/

ifeq ($(OS),Windows_NT)
	if [[ ! -f release/SDL3.dll ]]; then \
		$(CP) "C:\\msys64\\mingw64\\bin\\SDL3.dll" release/; \
	fi
	if [[ ! -f release/libwinpthread-1.dll ]]; then \
		$(CP) "C:\\msys64\\mingw64\\bin\\libwinpthread-1.dll" release/; \
	fi
else
	if [[ ! -f release/libvulkan.so ]]; then \
		$(CP) -L \
			$$(ldconfig -p | grep 'libvulkan.so ' | head -n 1 | awk '{print $$NF}') \
			release/; \
	fi
	if [[ ! -f release/libSDL3.so ]]; then \
		$(CP) -L \
			$$(ldconfig -p | grep 'libSDL3.so ' | head -n 1 | awk '{print $$NF}') \
			release/; \
	fi
endif

	cd release; $(VALGRIND_CALL) ./client


.PHONY: server
server:
	scons server -j $(shell nproc)
	./bin/server


.PHONY: test
test:
	scons test -j $(shell nproc)


.PHONY: cloc
cloc:
	cloc --skip-uniqueness $(shell find . -type f \
		\( -name "*.c" -o -name "*.h" -o -name "*.glsl" \
			-o -name "Makefile" -o -name "SConstruct" \) \
		! -path "*client/tex/*" ! -path "*client/font/*")
