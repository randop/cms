#!/usr/bin/env bash

set -euo pipefail

export MONGODBCDRIVER_VERSION="2.5.1"
export MONGODBCPPDRIVER_VERSION="4.5.1"
export PKG_CONFIG_PATH="/opt/mongo-c-driver/current/lib/pkgconfig"
export CMAKE_PREFIX_PATH="/opt/mongo-c-driver/current/lib/cmake:/opt/mongo-c-driver/current/lib/cmake/bson-${MONGODBCDRIVER_VERSION}:/opt/mongo-c-driver/current/lib/cmake/mongoc-${MONGODBCDRIVER_VERSION}"

MONGODBCPP_DIR="/opt/mongo-cpp-driver"
MONGODBCPP_CURRENT="${MONGODBCPP_DIR}/current"
MONGODBCPP_VERSION_DIR="${MONGODBCPP_DIR}/${MONGODBCPPDRIVER_VERSION}"
MONGODBCPP_BUILD_DIR="${MONGODBCPP_DIR}/${MONGODBCPPDRIVER_VERSION}/build"
if [ ! -d $MONGODBCPP_VERSION_DIR ]; then
  sudo rm -rf $MONGODBCPP_CURRENT
  sudo mkdir -p $MONGODBCPP_CURRENT
  sudo git clone \
    -b "r${MONGODBCPPDRIVER_VERSION}" \
    --depth 1 https://github.com/mongodb/mongo-cxx-driver.git ${MONGODBCPP_VERSION_DIR}
  sudo echo 'set(NEED_DOWNLOAD_C_DRIVER false CACHE INTERNAL "")' >"${MONGODBCPP_VERSION_DIR}/cmake/FetchMongoC.cmake"
  sudo mkdir -p ${MONGODBCPP_BUILD_DIR}
  cd $MONGODBCPP_BUILD_DIR
  sudo cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/opt/mongo-c-driver/current/lib/cmake \
    -DCMAKE_INSTALL_PREFIX=$MONGODBCPP_CURRENT \
    -DENABLE_TESTS=OFF \
    -DENABLE_EXAMPLES=OFF \
    -DENABLE_UNINSTALL=OFF \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_SHARED_AND_STATIC_LIBS=OFF \
    -DNEED_DOWNLOAD_C_DRIVER=OFF \
    -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations"
  sudo make -j$(nproc) && make install
  sudo echo "/opt/mongo-cpp-driver/lib" | sudo tee /etc/ld.so.conf.d/mongocpp-driver.conf
  sudo ldconfig
  sudo rm -fv /usr/lib/pkgconfig/libbsoncxx1-static.pc /usr/lib/pkgconfig/libmongocxx1-static.pc
  sudo ln -sv "$MONGODBCPP_CURRENT/lib/pkgconfig/libbsoncxx1-static.pc" /usr/lib/pkgconfig/libbsoncxx1-static.pc
  sudo ln -sv "$MONGODBCPP_CURRENT/lib/pkgconfig/libmongocxx1-static.pc" /usr/lib/pkgconfig/libmongocxx1-static.pc

  cat /usr/lib/pkgconfig/libmongocxx1-static.pc
fi
