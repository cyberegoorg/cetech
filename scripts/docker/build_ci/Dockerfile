FROM python:alpine

ENV TZ Europe/Prague

RUN apk add --no-cache \
            clang \
            cmake \
            libx11-dev \
            jemalloc-dev \
            mesa-dev \
            zlib-dev

ADD requirements.txt /tmp/requirements.txt
ADD dev_requirements.txt /tmp/dev_requirements.txt

RUN pip3 install -r /tmp/requirements.txt
RUN pip3 install -r /tmp/dev_requirements.txt