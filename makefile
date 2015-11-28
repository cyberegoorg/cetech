#########
# Utils #
#########
.PHONY: analyze
analyze:
	@echo Analyze...
	cppcheck --template=gcc --enable=all --inconclusive --std=posix ./engine/src/

.PHONY: scan-build32
scan-build32:
	scan-build -analyze-headers make -C ${ROOT_DIR} -j 4 linux-clang-debug32
	
.PHONY: scan-build64
scan-build64:
	scan-build -analyze-headers make -C ${ROOT_DIR} -j 4 linux-clang-debug64

.PHONY: uncrustify
uncrustify:
	@echo Uncrustify...
	-@find ./engine/src/ ./engine/tests/ -regex ".*\.\(c\|cc\|h\)" -print0 | xargs -0 uncrustify -l c -c ./scripts/uncrustify.cfg --no-backup
#########

linux:
	python ./make.py
