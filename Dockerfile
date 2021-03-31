FROM nvidia/cudagl:11.2.2-devel-ubuntu20.04 AS base
LABEL maintaner=otavio.b.gomes@gmail.com

RUN apt update && DEBIAN_FRONTEND=noninteractive apt -yq install aria2

WORKDIR /
RUN aria2c https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN bash Miniconda3-latest-Linux-x86_64.sh -b -p /miniconda3
ENV PATH="/miniconda3/bin/:${PATH}"
RUN rm Miniconda3-latest-Linux-x86_64.sh

ADD environment.yml /
RUN conda env update -n base --file environment.yml
RUN rm environment.yml

#####
# Trying image
##
FROM dev as try

ADD . /tensorviz
WORKDIR /tensorviz
RUN python setup.py install -j2

RUN mkdir /exec
WORKDIR /exec

ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display
