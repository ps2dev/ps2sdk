ARG BASE_DOCKER_IMAGE

FROM $BASE_DOCKER_IMAGE

COPY . /src

RUN apk add build-base git bash

# Still compilation is not fully compatible with multi-thread
RUN cd /src && \
    make -j $(getconf _NPROCESSORS_ONLN) clean && \
    make -j $(getconf _NPROCESSORS_ONLN) && \
    make -j $(getconf _NPROCESSORS_ONLN) install
# Create symbolink links using relative paths
RUN (cd $PS2DEV/ee/mips64r5900el-ps2-elf/lib && ln -sf ../../../ps2sdk/ee/lib/libcglue.a libcglue.a && cd -)
RUN (cd $PS2DEV/ee/mips64r5900el-ps2-elf/lib && ln -sf ../../../ps2sdk/ee/lib/libpthreadglue.a libpthreadglue.a && cd -)
RUN (cd $PS2DEV/ee/mips64r5900el-ps2-elf/lib && ln -sf ../../../ps2sdk/ee/lib/libkernel.a libkernel.a && cd -)
RUN (cd $PS2DEV/ee/mips64r5900el-ps2-elf/lib && ln -sf ../../../ps2sdk/ee/lib/libcdvd.a libcdvd.a && cd -)

# Second stage of Dockerfile
FROM alpine:latest

ENV PS2DEV /usr/local/ps2dev
ENV PS2SDK $PS2DEV/ps2sdk
ENV PATH $PATH:${PS2DEV}/bin:${PS2DEV}/ee/bin:${PS2DEV}/iop/bin:${PS2DEV}/dvp/bin:${PS2SDK}/bin

COPY --from=0 ${PS2DEV} ${PS2DEV}
