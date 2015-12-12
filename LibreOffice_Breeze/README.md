# LibreOffice icon theme
This folder contains sources for LibreOffice Breeze icon theme.
The Makefile builds the default theme and another one compatible with dark UIs into each zip files ready for use in LibreOffice.

## Create icon theme files
To build both icon theme files, just run the following command from this directory:

`make`

Once it finishes successfully, you should see the following zip files: 

* images_breeze.zip
* images\_breeze_dark.zip

To start using them, you just have to copy them to `<INSTALLDIR>/share/config/` (e.g.: `/usr/lib/libreoffice/share/config/`) and you should be able to select them in LibreOffice configuration: `Tools->Options->View`

## Requirements
1. GNU Make
2. Imagemagick
3. Zip

