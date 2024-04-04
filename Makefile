all: Release/iimages

# Not needed apparently: -lopencv_imgproc -lopencv_video
Release/iimages: ifeatures.cpp ifeatures.h main.cpp
	g++ -o Release/iimages \
		ifeatures.cpp main.cpp \
		-Wl,--copy-dt-needed-entries \
		-I /usr/include/opencv4 \
		-lopencv_core -lopencv_highgui -lopencv_videoio

clean:
	rm -f Release/iimages

# Some of these might be not required.
deps_ubuntu2204:
	sudo apt-get install \
		libopencv-core-dev    libopencv-core4.5d \
		libopencv-highgui-dev libopencv-highgui4.5d \
		libopencv-imgproc-dev libopencv-imgproc4.5d \
		libopencv-video-dev   libopencv-video4.5d \
		libopencv-videoio-dev libopencv-videoio4.5d 
