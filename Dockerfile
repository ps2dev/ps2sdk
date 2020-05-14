FROM fjtrujy/ps2dev:toolchain-latest

COPY . /src/ps2sdk

RUN \
  apk add --no-cache --virtual .build-deps gcc musl-dev && \
  cd /src/ps2sdk && \
  make && \
  make install && \
  make clean && \
  apk del .build-deps && \
  rm -rf \
    /src/* \
    /tmp/*

WORKDIR /src
