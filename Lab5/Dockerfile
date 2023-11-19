FROM ubuntu:20.04

# Install necessary packages
RUN apt-get update && apt-get install -y build-essential m4

# Download and install bison 3.8.2
RUN apt-get -y install bison flex

COPY . lab/

RUN cd /lab && make calculator

