Cairo is a 2d graphics library, which supports drawing on SVG files.
I suggest reading more about cairo here: http://cairographics.org/tutorial/ and here: http://cairographics.org/samples/
You can find the API somewhere on that site, too.

Installation[short]:
-Include the "include" folder, which contains cairo's header files inside the "cairo" folder
-Include the "lib" folder, add "cairo.lib" dependency
- Include the "cairo.dll":
	-Option 1: add "PATH=%PATH%;$(ProjectDir)lib" to your environment
	-Option 2: copy "cairo.dll" to the projects output folder

Installation[long] (for Visual Studio 2015, but should work for older version too):
1. Open Project Properties
2. Open "C/C++" -> "General", under "Additional Include Directories", add "$(ProjectDir)include"
3. Open "Linker" -> "General", under "Additional Library Directories", add "$(ProjectDir)lib"
4. Open "Linker" -> "Input", under "Additional Dependencies", add "cairo.lib"
5. Use either option:
	5.1. Move/Copy "$(ProjectDir)\lib\cairo.dll" to your working directory, usually in the folder before "\lib" where all the source files are
	5.2. Open "Configuration Properties" -> "Debugging", under Environment, add "PATH=%PATH%;$(ProjectDir)lib"
	

In your source code, if you want to use cairo (with svg support), add
	#include <cairo\cairo.h>
	#include <cairo\cairo-svg.h>


#### Appendix: I pulled the (x64) compiled libraries and header files from https://github.com/tordex/cairo , if you use a x86 environment, you should get the correct libraries from there.