SCSS = $(wildcard css/*.scss)
JS   = $(wildcard js/*.js)

CSS  = $(SCSS:.scss=.css)
HASH = $(CSS) $(JS)

ALL = $(CSS) $(HASH:%=%.hash)

all:  $(ALL)

%.css: %.scss
	sass $< > $@

%.hash: %
	git hash-object $< > $@

clean:
	git clean -fX
