FROM fjtrujy/ps2dev:toolchain-latest

COPY . /src/ps2sdk

RUN \
  apk add --no-cache --virtual .build-deps gcc musl-dev && \
  cd /src/ps2sdk && \
  make && \
  make install && \
  make clean && \
  ln -sf "$PS2SDK/ee/lib/libps2sdkc.a" "$PS2DEV/ee/ee/lib/libps2sdkc.a" && \
  ln -sf "$PS2SDK/ee/lib/libkernel.a"  "$PS2DEV/ee/ee/lib/libkernel.a" && \
  apk del .build-deps && \
  rm -rf \
    /src/* \
    /tmp/*

WORKDIR /src
