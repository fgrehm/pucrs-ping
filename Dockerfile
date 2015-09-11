FROM ubuntu:14.04
RUN apt-get update \
    && apt-get install -y make gcc build-essential \
    && apt-get clean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/*
