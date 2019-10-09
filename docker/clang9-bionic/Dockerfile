FROM ubuntu:18.04

ENV LANG                C.UTF-8
ENV LC_ALL              C.UTF-8
ENV CLANG_HOME          /clang_9.0.0
ENV CLANG_DIR           clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04

RUN apt-get update \
 && apt-get install -y \
        build-essential \
        cmake \
        xz-utils \
 && rm -Rf /var/lib/apt/lists/*

RUN wget -O- http://releases.llvm.org/9.0.0/$CLANG_DIR.tar.xz \
  | tar -xJC / \
 && ln -s /$CLANG_DIR $CLANG_HOME

ENV PATH                $CLANG_HOME/bin:$PATH
ENV LD_LIBRARY_PATH     $CLANG_HOME/lib:$LD_LIBRARY_PATH
ENV CC                  clang
ENV CXX                 clang++
ENV CXXFLAGS            -std=c++17 -stdlib=libc++

CMD ["/bin/sh"]
