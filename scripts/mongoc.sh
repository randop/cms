#!/usr/bin/env bash
sudo apt install -y git meson
mkdir -p /tmp/mongo-c-driver/current
git clone -b 2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /tmp/mongo-c-driver/2.0.2
cd /tmp/mongo-c-driver/2.0.2 && cmake -DCMAKE_INSTALL_PREFIX=/tmp/mongo-c-driver/current .
cd /tmp/mongo-c-driver/2.0.2 && make all install