CC    := gcc
LD    := gcc
CFLGS := -g -Wall -Wpedantic -Wextra
FILEN := bfint

PREFIX = /usr/local

# link
$(FILEN): obj/main.o
	$(LD) $(LFLGS) $(OLDIRS) $(OLIB) -o $@ $^

# the | means, that objdirs target should be existent, not more recent
# build the objects
obj/main.o: src/main.c | objdirs
	$(CC) $(CFLGS) $(OIDIRS) -c $< -o $@

install: $(FILEN)
	$(shell cp $(FILEN) $(PREFIX)/bin/$(FILEN))

## support

# clean the built files
clean:
	$(RM) -r ./obj/
	$(RM) $(FILEN)

# check that object output directories exist
objdirs:
	@mkdir -p ./obj/

# PHONY: run every time
.PHONY: clean

