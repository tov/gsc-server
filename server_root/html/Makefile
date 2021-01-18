COURSE   ?= cs211
DEST_DIR ?= ~gsc/$(COURSE)/html

SCSS = $(wildcard css/*.scss)
JS   = $(wildcard js/*.js)

CSS  = $(SCSS:.scss=.css)
HASH = $(CSS) $(JS)

ALL = $(CSS) $(HASH:%=%.hash)

all:  $(ALL)

install: $(ALL)
	@set -e; \
	for f in $^; do \
	    install -dvm755 $(DEST_DIR)/$$(dirname $$f); \
	    install -Cvm644 $$f $(DEST_DIR)/$$f; \
	done

%.css: %.scss
	sass $< > $@ --style compressed

js/%.js: js/%.ts
	cd js && yarn build
	touch $@

%.hash: %
	git hash-object $< > $@

clean:
	git clean -fX

.PHONY: all clean
