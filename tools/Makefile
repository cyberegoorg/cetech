UIS = $(shell find src -name '*.ui')

all: $(UIS)
	@python -c "import PyQt5.uic;PyQt5.uic.compileUiDir('src/', recurse=True)"

clean: $(UIS:.ui=.py)
	@rm $<