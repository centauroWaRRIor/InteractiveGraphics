Step to correctly deploy the utilities:

1. Put is anywhere you want, and write down the absolute path name, for example:
   You put everything extracted (shall be 2 folders and 1 readme (this file) ) in a folder named OpenGL, and put this OpenGL folder on root directory of C:\
   Then your Absolute path to these files is C:\OpenGL

2. In VS2010, click Project tab, and select the last one (shall be code Properties if you are using 8.29 code)
3. Expand COngiguration Properties
4. Expand C/C++ tab, and in the right side window, you'll see "Additional Include Directories"
   Change the value to "C:\OpenGL"\include, the quoted part is the exact copy of what you have in step one
5. Click on Linker tab, you'll find: "Addtional Library Directories"
   Change the value to "C:\OpenGL"\lib, the quoted part is same as step 4
6 Build the project again

If there are unresolved link, try to put shese codes at the beginning of Scene.h, right after the first line "#pragma once":

  #pragma comment(lib,"fltk.lib")
#pragma comment(lib,"fltkgl.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"libtiff.lib")