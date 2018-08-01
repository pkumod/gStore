#This Dockerfile is just a primitive one. We welcome optimization and simplification.

#Attention: 
#It is best to update the process of modification in the rear of the original code(reduce the order changes).
#Otherwise the speed of construction will be influenced on a large scale.

#TODO: some space can be saved by using the low version of gcc mirror(e.g-gcc:5).
#But its mobility and dependence has not been tested yet and waiting for confirmation.

FROM gcc:8

#download all the optional installation library in gstore's document.
RUN apt-get update && apt-get install -y --no-install-recommends realpath \
         ccache \
         openjdk-8-jdk \
         libreadline-dev \
         libboost-all-dev \
         && rm -rf /var/lib/apt/lists/

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

#ENV CC="ccache gcc" CXX="ccache g++"? should not be choosed.
#Because it will trigger extremely troublesome bugs and its reason is still waiting for checking.

#The solution to the problem of java, whose default setting is using ansii to encode.
ENV LANG C.UTF-8


#Make compiling will be executed or replaced automatically when the container launch. 
#notice: RUN make still has some problems now. 

#Subsequent test version can be put here.
#And some procedures you need can also be attached here.
#CMD ["make"]
RUN make
