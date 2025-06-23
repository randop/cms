#!/usr/bin/env bash
mkdir -p /opt/mongo-c-driver/current
git clone -b v2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/2.0.2
cd /opt/mongo-c-driver/2.0.2 && cmake -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current .
cd /opt/mongo-c-driver/2.0.2 && make all install
echo "/opt/mongo-c-driver/current" > /etc/ld.so.conf.d/boost.conf
echo "Linking mongoc..." && ldconfig