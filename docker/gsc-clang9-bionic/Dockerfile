FROM wt-clang9-bionic

ENV GSC_BRANCH          openam
ENV GSC_PKG             gsc-server-$GSC_BRANCH
ENV GSC_REPO_URI        https://github.com/tov/gsc-server/
ENV GSC_SRC_URL         $GSC_REPO_URI/archive/$GSC_BRANCH.tar.gz

RUN cd /tmp \
 && curl -SL $GSC_SRC_URL | tar zx \
 && mkdir -p $GSC_PKG/build \
 && cd $GSC_PKG/build \
 && cmake .. -DCMAKE_BUILD_TYPE=Release \
 && make -j 4 gscd-fcgi gsc-auth \
 && install -m 755 gscd-fcgi gsc-auth /usr/bin \
 && cd /tmp \
 && rm -Rf $GSC_PKG

CMD ["/bin/sh"]
