ARG BASE_DOCKER_IMAGE

FROM $BASE_DOCKER_IMAGE

COPY . /src

RUN apk add build-base
RUN cd /src && make all install clean
RUN ln -sf "$PS2SDK/ee/lib/libps2sdkc.a" "$PS2DEV/ee/ee/lib/libps2sdkc.a"
RUN ln -sf "$PS2SDK/ee/lib/libkernel.a"  "$PS2DEV/ee/ee/lib/libkernel.a"

# Second stage of Dockerfile
FROM alpine:latest  

ENV PS2DEV /usr/local/ps2dev
ENV PS2SDK $PS2DEV/ps2sdk
ENV PATH $PATH:${PS2DEV}/bin:${PS2DEV}/ee/bin:${PS2DEV}/iop/bin:${PS2DEV}/dvp/bin:${PS2SDK}/bin

COPY --from=0 ${PS2DEV} ${PS2DEV}