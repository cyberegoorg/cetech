##################################
# Makefile helper.               #
# Used as Kdevelop root makefile #
##################################


linux64-clang:
	python ./make.py
	.build/linux64_clang/bin/cetech_testDebug -a

clean:
	python ./make.py clean

#########
# Utils #
#########
.PHONY: analyze
analyze:
	@echo Analyze...
	cppcheck --std=c++11 --template=gcc --enable=all --inconclusive --std=posix engine/src/ -I engine/src/ -I external/.build/linux64/include -I external/.build/linux64/include -UCETECH_TEST -j 4 -i external/.build/linux64/include/catch -i engine/src/tests/

.PHONY: scan-build64
scan-build64: clean
	scan-build -analyze-headers make -j 4 linux

.PHONY: uncrustify
uncrustify:
	@echo Uncrustify...
	-@find ./engine/src/ ./engine/tests/ -regex ".*\.\(c\|cc\|h\)" -print0 | xargs -0 uncrustify -l c -c ./scripts/uncrustify.cfg --no-backup
#########

