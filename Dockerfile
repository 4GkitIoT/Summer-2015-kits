FROM ubuntu:14.04.3 
MAINTAINER Krzysztof Korycki krzysztof.korycki@orange.com

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y wget supervisor unzip git \
	lua5.2 lua5.2-dev cmake g++ git libssl-dev unzip
RUN cd /root \
    && mkdir 4gkit

#####
# BUILD AND INSYALL DEPENDENCIES
#####

ADD ./ /root/4gkit
RUN chmod +x /root/4gkit/install.sh
RUN cd /root/4gkit \
	&& ./install.sh

#####
# GENERATE FAKE ID OF CLIENT
#####

RUN wget bizon.sdqac.rd.tp.pl/fakeid.lua
RUN lua fakeid.lua 123456789012

#####
# INSTALL SSH AND SUPERVISOR
#####

RUN apt-get install -y openssh-server
RUN mkdir -p /var/run/sshd /var/log/supervisor
RUN echo 'root:root' |chpasswd
RUN sed -ri 's/^PermitRootLogin\s+.*/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -ri 's/UsePAM yes/#UsePAM yes/g' /etc/ssh/sshd_config

RUN echo "[supervisord] \n\
nodaemon=true \n\
[program:sshd] \n\
command=/usr/sbin/sshd -D \n\
[program:4gkit] \n\
command=/bin/bash -c 'cd /root/4gkit/Core;./StreamZCore'" > /etc/supervisor/conf.d/supervisord.conf


VOLUME ["/root/4gkit/modules"]

EXPOSE 22 
CMD ["/usr/bin/supervisord"]

