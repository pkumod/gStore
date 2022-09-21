FROM lsvih/gcc-cmake-boost:v1
RUN apt -qqy update \
	&& apt install -qqy --no-install-recommends \
	libssl-dev libcurl4-openssl-dev libreadline-dev uuid-dev \
	&& ldconfig -v \
	&& echo "*    -    nofile    65535" >> /etc/security/limits.conf \
	&& echo "*    -    noproc    65535" >> /etc/security/limits.conf

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

ENV LANG C.UTF-8

RUN make pre -j && make -j FIRST_BUILD=y \
	&& apt autoclean && apt clean \
	&& rm -rf /usr/src/gstore/.git \
	&& rm -rf /usr/src/gstore/tools \
	&& rm -rf /tmp/* /var/tmp/* \
	&& rm -rf /usr/share/doc/* \
	&& rm -rf /var/lib/apt/lists/*