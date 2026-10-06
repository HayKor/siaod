SRC = $(wildcard src/*.typ)
PDF = $(SRC:src/%.typ=build/%.pdf)

all: $(PDF)

build/%.pdf: src/%.typ include/settings.typ
	typst compile --root .. $< $@

clean:
	rm -f build/*.pdf

.PHONY: all clean
