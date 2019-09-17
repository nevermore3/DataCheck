FROM ubuntu:16.04

MAINTAINER pemario "jianghang@kuandeng.com"

RUN apt update
RUN apt-get install -y vim python3 python2.7

RUN mkdir -p /build/link
RUN ln -s /build/link/data /build/data

#复制配置文件到相应位置
ADD build build
RUN ln -s /build/bin/libgeos.so /build/bin/libgeos.so.3.6.2
WORKDIR /build/bin
