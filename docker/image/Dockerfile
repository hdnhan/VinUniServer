FROM ubuntu:22.04

# setup a user as sudo
RUN apt-get update
RUN apt-get install -y openssh-server
RUN apt-get install -y vim tmux

RUN apt-get install -y sudo
RUN useradd -ms /bin/bash ubuntu
RUN adduser ubuntu sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >>/etc/sudoers
USER ubuntu
WORKDIR /home/ubuntu

# service ssh start
ENTRYPOINT sudo service ssh restart && bash
RUN sudo sed -i 's/#\?PasswordAuthentication yes/PasswordAuthentication no/' /etc/ssh/sshd_config
RUN mkdir -p /home/ubuntu/.ssh
RUN touch /home/ubuntu/.ssh/authorized_keys

# enable reboot command
RUN echo '#!/bin/bash\nkill -15 1' | sudo tee /usr/bin/reboot
RUN sudo chmod +x /usr/bin/reboot                # reboot
RUN sudo ln -sf /usr/bin/reboot /usr/sbin/reboot # sudo reboot

# install miniconda
ENV PATH="/home/ubuntu/miniconda3/bin:$PATH"
RUN wget -q https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN bash Miniconda3-latest-Linux-x86_64.sh -b
RUN rm -f Miniconda3-latest-Linux-x86_64.sh
RUN conda init
RUN conda config --set auto_activate_base false
RUN conda update -n base -c defaults conda -y

# install deep learning frameworks
COPY ./pytorch.yml /home/ubuntu/pytorch.yml
RUN conda env create -f /home/ubuntu/pytorch.yml

COPY ./tf2.yml /home/ubuntu/tf2.yml
RUN conda env create -f /home/ubuntu/tf2.yml
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/miniconda3/envs/tf2/lib/

# COPY ./tf1.yml /home/ubuntu/tf1.yml
# RUN conda env create -f /home/ubuntu/tf1.yml

RUN rm -f /home/ubuntu/*.yml

# display a message
COPY 00-header /etc/update-motd.d
# env when ssh
RUN echo PATH=$PATH | sudo tee /etc/environment
RUN echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH | sudo tee -a /etc/environment
