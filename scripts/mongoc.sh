#!/usr/bin/env bash

# IMPORTANT: Github ubuntu runner uses proprietary azure that has poor limitations and restrictions
# Refer at https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners

export MONGODBCDRIVER_VERSION="2.3.2"
sudo mkdir -p /opt/mongo-c-driver/current
sudo git clone \
  -b ${MONGODBCDRIVER_VERSION} \
  --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}
sudo sed -i "/#if (OPENSSL_VERSION_NUMBER >= 0x10001000L)/i #define OPENSSL_NO_OCSP 1" /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}/src/libmongoc/src/mongoc/mongoc-openssl-private.h
sudo sed -i 's/MONGOC_ERROR("Could not set cipher list for TLSv1.2 and below: %s", ERR_STR)/char _err_buf[256]; ERR_error_string_n(ERR_get_error(), _err_buf, sizeof(_err_buf)); MONGOC_ERROR("Could not set cipher list for TLSv1.2 and below: %s", _err_buf)/' /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}/src/libmongoc/src/mongoc/mongoc-openssl.c
sudo mkdir -p /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}/build
cd /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}/build
sudo cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current \
  -DENABLE_TESTS=OFF \
  -DENABLE_EXAMPLES=OFF
sudo make install
sudo echo "/opt/mongo-c-driver/current/lib" > /etc/ld.so.conf.d/mongoc-driver.conf
sudo ldconfig
sudo rm -fv /usr/lib/pkgconfig/mongoc.pc /usr/lib/pkgconfig/bson.pc
sudo rm -fv /usr/lib/pkgconfig/mongoc-static.pc /usr/lib/pkgconfig/bson-static.pc
sudo rm -fv /usr/lib/pkgconfig/mongoc2.pc /usr/lib/pkgconfig/bson2.pc
sudo rm -fv /usr/lib/pkgconfig/mongoc2-static.pc /usr/lib/pkgconfig/bson2-static.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2.pc /usr/lib/pkgconfig/mongoc.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2.pc /usr/lib/pkgconfig/mongoc2.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2-static.pc /usr/lib/pkgconfig/mongoc2-static.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson2.pc
sudo ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson2-static.pc