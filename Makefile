
CC=cc
PREFIX=/usr/local
RAGEL=/usr/local/bin/ragel
DYLIB=obj/libreadstat.dylib
MIN_OSX=10.9

all:
	@mkdir -p obj
	[ -x $(RAGEL) ] && $(RAGEL) src/readstat_por_parse.rl -G2
	[ -x $(RAGEL) ] && $(RAGEL) src/readstat_sav_parse.rl -G2
	[ -x $(RAGEL) ] && $(RAGEL) src/readstat_spss_parse.rl -G2
	$(CC) -Os src/*.c -dynamiclib -o $(DYLIB) -I/usr/local/include -llzma -lz -liconv -Wall -Wno-multichar -Werror -pedantic -mmacosx-version-min=$(MIN_OSX) -DHAVE_LZMA
	$(CC) -Os src/bin/*.c -o obj/readstat -Lobj -lreadstat -Isrc

install: all
	@mkdir -p $(PREFIX)/lib
	@cp $(DYLIB) $(PREFIX)/lib/
	@mkdir -p $(PREFIX)/include
	@cp src/readstat.h $(PREFIX)/include/

clean:
	rm -rf obj
