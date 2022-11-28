ARG BASE_DOCKER_IMAGE

FROM $BASE_DOCKER_IMAGE

COPY . /src

RUN apk add build-base git bash
RUN cd /src && \
    make -j $(getconf _NPROCESSORS_ONLN) && \
    make -j $(getconf _NPROCESSORS_ONLN) install
RUN ln -sf "$PS2SDK/ee/lib/libcglue.a" "$PS2DEV/ee/mips64r5900el-ps2-elf/lib/libcglue.a"
RUN ln -sf "$PS2SDK/ee/lib/libpthreadglue.a" "$PS2DEV/ee/mips64r5900el-ps2-elf/lib/libpthreadglue.a"
RUN ln -sf "$PS2SDK/ee/lib/libkernel.a"  "$PS2DEV/ee/mips64r5900el-ps2-elf/lib/libkernel.a"
RUN ln -sf "$PS2SDK/ee/lib/libcdvd.a"  "$PS2DEV/ee/mips64r5900el-ps2-elf/lib/libcdvd.a"

# Second stage of Dockerfile
FROM alpine:latest

ENV PS2DEV /usr/local/ps2dev
ENV PS2SDK $PS2DEV/ps2sdk
ENV PATH $PATH:${PS2DEV}/bin:${PS2DEV}/ee/bin:${PS2DEV}/iop/bin:${PS2DEV}/dvp/bin:${PS2SDK}/bin

COPY --from=0 ${PS2DEV} ${PS2DEV}
