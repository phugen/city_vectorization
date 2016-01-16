# city_vectorization

###Dependencies:
* [Cairo:](http://cairographics.org/) see [readme_cairo_usage.md](readme_cairo_usage.md) to make cairo work with your project
* [Opencv](http://opencv.org/downloads.html)
* [Qt](http://www.qt.io/download-open-source/), Visual Studio users: we recommend installing [Qt5-vs-addin](http://download.qt.io/official_releases/vsaddin/) additionally


###Opening the Visual Studio Project File
(tested with Visual Studio 2013)
* Install opencv inside the `opencv` folder, so that it contains the folders `build` and `sources`
  * Alternatively skip this operation until the end of this list, then change the project settings so that it additionally uses the path of your opencv folders (`lib` and `include` from opencv). Howevery, if you plan to do this, make sure you don't commit an invalid project file. Not everyone uses the same paths
* Make sure your Qt5 Addin settings are correct
* Open `city_vectorization.vcxproj` with Visual Studio
* Change Configuration to `Release` and Platform to `x64`
  * (if you use `x86` Platform you can copy most of the settings from `x64`, they just are not preconfigured currently)
* Make sure the `Qt Project Settings` are using the correct `Version`
* Open `Project Properties -> Configuration Properties`
  * Under `Debugging -> Environment` delete the preconfigured stuff of Qt and add `PATH=%QTDIR%\bin;$(ProjectDir)lib;%PATH%;`
    (usually the preconfig of Qt doesn't work, even though it tries to do the same thing we added just now)
* The Project should now be ready to compile and run

