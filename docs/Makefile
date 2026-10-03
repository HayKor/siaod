SRC = $(wildcard src/*.typ)
PDF = $(SRC:src/%.typ=output/%.pdf)

all: $(PDF)

output/%.pdf: src/%.typ include/settings.typ
	typst compile --root .. $< $@

clean:
	rm -f output/*.pdf

.PHONY: all clean
