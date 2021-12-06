FROM nvidia/cudagl:11.2.2-devel-ubuntu20.04 AS base
LABEL maintaner=otavio.b.gomes@gmail.com

RUN apt update && DEBIAN_FRONTEND=noninteractive apt -yq install wget

WORKDIR /
RUN wget https://repo.anaconda.com/miniconda/Miniconda3-py39_4.10.3-Linux-x86_64.sh
RUN bash Miniconda3-py39_4.10.3-Linux-x86_64.sh -b -p /miniconda3 && rm Miniconda3-py39_4.10.3-Linux-x86_64.sh
ENV PATH="/miniconda3/bin/:${PATH}"

ADD environment.yml /
RUN conda env update -n base --file environment.yml && rm environment.yml
RUN conda clean --all -y
RUN rm -r /miniconda3/lib/libcuda* /miniconda3/lib/libcuf* /miniconda3/lib/libcub* /miniconda3/lib/libcuso*\
    /miniconda3/lib/libcurand* /miniconda3/pkgs/cudatoolkit* /miniconda3/pkgs/cudnn*

####
# Devcontainer image
##
FROM base as devcontainer
LABEL maintaner=otavio.b.gomes@gmail.com

RUN apt update && DEBIAN_FRONTEND=noninteractive apt -yq install sudo git\
    byobu bash-completion curl doxygen graphviz

RUN pip install -U pip
ADD requirements-dev.txt .
RUN pip install -r requirements-dev.txt && rm requirements-dev.txt

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

RUN chmod 777 /miniconda3/bin /miniconda3/lib/python3.9/site-packages

USER $USERNAME

SHELL ["/bin/bash", "-c"]

ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display

#####
# Builder image
##
FROM base as builder

ADD . /tensorviz
WORKDIR /tensorviz
RUN python setup.py bdist_wheel 

#####
# Trying image
##
FROM nvidia/cudagl:11.2.2-runtime-ubuntu20.04 AS try

COPY --from=base /miniconda3 /miniconda3
ENV PATH="/miniconda3/bin/:${PATH}"

WORKDIR /exec

COPY --from=builder /tensorviz/dist/*.whl .
RUN pip install *.whl && rm *.whl

COPY --from=base /usr/lib/x86_64-linux-gnu/libOpenGL.so.0 /usr/lib/x86_64-linux-gnu/libOpenGL.so.0

ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display
