#!/usr/bin/env bash

# IMPORTANT: Github ubuntu runner uses proprietary azure that has poor limitations and restrictions
# Refer at https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners

sudo mkdir -p /opt/mongo-c-driver/current
sudo git clone -b 2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/2.0.2
cd /opt/mongo-c-driver/2.0.2
sudo cmake -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current .
sudo make all install