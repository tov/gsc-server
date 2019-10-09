REGISTRY    = jessetov
DOCKERFILES = $(shell find * -type f -name Dockerfile)
IMAGES      = $(subst /,-,$(subst /Dockerfile,,$(DOCKERFILES)))
DEPENDS     = .depends.mk

.PHONY: all clean $(IMAGES)

all: $(IMAGES)

clean:
	rm -f $(DEPENDS)

$(DEPENDS): $(DOCKERFILES) Makefile
	grep '^FROM $(REGISTRY)/' $(DOCKERFILES) | \
	  sed 's@/Dockerfile@@;s@:FROM $(REGISTRY)/@: @' > $@

sinclude $(DEPENDS)

$(IMAGES): %:
	docker build -t $(REGISTRY)/$@ $@
	docker image tag $(REGISTRY)/$@ $@

