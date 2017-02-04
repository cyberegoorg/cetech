FROM ubuntu:latest

ENV TZ Europe/Prague

RUN apt-get update
RUN apt-get install -y clang \
                        cmake \
                        gcc \
                        g++ \
                        libx11-dev \
                        libjemalloc-dev \
                        libgl1-mesa-dev \
                        zlib1g-dev \
                        python3-pip

ADD requirements.txt /tmp/requirements.txt
ADD dev_requirements.txt /tmp/dev_requirements.txt

RUN pip3 install -r /tmp/requirements.txt -r/tmp/dev_requirements.txt
