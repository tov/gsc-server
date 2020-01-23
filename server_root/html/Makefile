ALL = css/gsc.css \
      css/gsc.css.hash \
      gsc.js.hash

all:  $(ALL)

%.css: %.scss
	sass $< > $@

%.hash: %
	git hash-object $< > $@

clean:
	git clean -fX
