License: GPL

Installation on Mac
1. Download Develop Kit for Mac.
2. qmake -project
3. In .pro file, add RC_FILE = Images/Glasses.icns
4. Also add LIBS += -lz
5. qmake -spec macx-g++
6. make

cp Images/BAM.icns Images/SAM.icns BAMseek.app/Contents/Resources/
macdeployqt BAMseek.app -dmg
