#
# This Makefile builds icon packages ready to be copied to <INSTALLDIR>/share/config/
# (e.g.: /usr/lib/libreoffice/share/config).
#

SRC_DIR=breeze
LIGHT_DIR=breeze_light
DARK_DIR=breeze_dark

all: images_breeze.zip images_breeze_dark.zip

# PNG conversion is done in 2 steps:
#     1. SVG files are converted and copied
#     2. All other files are copied
%.png: breeze
	mkdir -p $(LIGHT_DIR)
	mkdir -p $(DARK_DIR)
	cd $(SRC_DIR) && \
	for f in `find . -name "*.svg"`; do \
		newf_light="../$(LIGHT_DIR)/`echo $$f | sed 's#.svg#.png#g'`"; \
		newf_dark="../$(DARK_DIR)/`echo $$f | sed 's#.svg#.png#g'`"; \
		mkdir -p `dirname $$newf_light` `dirname $$newf_dark`; \
		convert -background none -size 16x16 "svg:$$f" "png:$$newf_light"; \
		cat "$$f" | sed -e 's/#4d4d4d/#f2f2f2/gI' | sed -e 's/#adadad/#f9f9f9/gI' \
		    | convert -background none -size 16x16 svg:- "png:$$newf_dark"; \
		chmod 644 "$$newf_light" "$$newf_dark"; \
	done && \
	for f in `find . -not -type d -not -name "*.svg"`; do \
		newf_light="../$(LIGHT_DIR)/$$f"; \
		newf_dark="../$(DARK_DIR)/$$f"; \
		mkdir -p "`dirname $$newf_light`" "`dirname $$newf_dark`"; \
		cp -L "$$f" "$$newf_light"; \
		cp -L "$$f" "$$newf_dark"; \
		chmod 644 "$$newf_light" "$$newf_dark"; \
	done && \
	echo "Performing PNG rendering and dark theme color transformations/rendering..."
	
images_breeze.zip images_breeze_dark.zip: %.png
	cd $(LIGHT_DIR) && zip -y -q -r ../images_breeze.zip *
	cd $(DARK_DIR) && zip -y -q -r ../images_breeze_dark.zip *

clean:
	rm -rf $(LIGHT_DIR)
	rm -rf $(DARK_DIR)
	rm -f images_breeze.zip 
	rm -f images_breeze_dark.zip


