FROM ps2dev/ps2toolchain

ENV PS2SDK $PS2DEV/ps2sdk
ENV PATH   $PATH:$PS2SDK/bin

COPY . /ps2sdk

RUN make install -C /ps2sdk \
    && cd /ps2sdk \
    && make install \
    && rm -rf \
        /ps2sdk \
        $PS2SDK/test.tmp
