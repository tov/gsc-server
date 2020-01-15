all: gsc.css

%.css: %.scss
	sass $< > $@
	git hash-object $@ > $@.hash

clean:
	git clean -fX
