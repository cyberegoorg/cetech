## SharpBgfx

Provides managed (C#,VB,F#,etc) bindings for the bgfx graphics library.

See <https://github.com/bkaradzic/bgfx>.

The main library is a minimal set of pinvoke declarations. The easiest way to use it is to drop the amalgamated SharpBgfx.cs file into your project and go. Alternatively, you can build the library into a managed assembly.

### Platforms

Currently only tested on Windows, though it will probably run fine on Mac and Linux if Mono is installed and the bgfx native library is rebuilt for those platforms.
