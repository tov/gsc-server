FROM clang9-bionic

ENV WT_VERSION            4.1.1
ENV WT_PKG                wt-$WT_VERSION
ENV WT_REPO_URI           https://github.com/emweb/wt/archive
ENV WT_SRC_URL            $WT_REPO_URI/$WT_VERSION.tar.gz

RUN apt-get update \
 && apt-get install -y \
        libboost-all-dev \
        libfcgi-dev \
        libpq-dev \
 && rm -Rf /var/lib/apt/lists/*

RUN cd /tmp \
 && curl -SL $WT_SRC_URL | tar zx \
 && cd $WT_PKG \
 && mkdir build \
 && cd build \
 && cmake .. \
        -DCONNECTOR_HTTP=Off \
        -DBUILD_EXAMPLES=Off \
        -DBUILD_TESTS=Off \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_STANDARD=17 \
 && make -j 4 \
 && make install \
 && cd /tmp \
 && rm -Rf $WT_PKG

CMD ["/bin/sh"]
