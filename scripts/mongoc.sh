#!/usr/bin/env bash

# IMPORTANT: Github ubuntu runner uses proprietary azure that has poor limitations and restrictions
# Refer at https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners

sudo apt install -y git meson libboost-all-dev
mkdir -p /tmp/mongo-c-driver/current
git clone -b 2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /tmp/mongo-c-driver/2.0.2
cd /tmp/mongo-c-driver/2.0.2 && cmake -DCMAKE_INSTALL_PREFIX=/tmp/mongo-c-driver/current .
cd /tmp/mongo-c-driver/2.0.2 && make all install

export LD_LIBRARY_PATH=/usr/local/lib:/tmp/mongo-c-driver/current/lib

echo "/tmp/mongo-c-driver/current" | sudo tee -a /etc/ld.so.conf.d/mongo.conf > /dev/null

sudo ldconfig