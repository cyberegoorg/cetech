UIS = $(shell find src -name '*.ui')
QRCS = $(shell find src -name '*.qrc')

all: $(UIS) $(QRCS)
	@python -c "import PyQt5.uic;PyQt5.uic.compileUiDir('src/', recurse=True)"
	@pyrcc5 src/cetech/playground/qt/rc/playground.qrc -o src/cetech/playground/qt/rc/playground.py

clean: $(UIS:.ui=.py) $(QRCS:.qrc=.py)
	@rm $<