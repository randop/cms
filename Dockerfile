# Cross-platform docker image builder
# Build stage: Use pre-built Boost C++ image
FROM rfledesma/boost:latest AS builder

ARG MONGODBCDRIVER_VERSION=2.2.4
ARG MONGODBCXXDRIVER_VERSION=4.2.0

ENV MONGODB_DRIVER_VERSION=$MONGODBCDRIVER_VERSION
ENV MONGODBCXX_DRIVER_VERSION=$MONGODBCXXDRIVER_VERSION

WORKDIR /app
COPY . .

# Install development dependencies
RUN echo "Installing development packages..." && \
    DEBIAN_FRONTEND=noninteractive apt update -qqq && \
    DEBIAN_FRONTEND=noninteractive apt install -qqq -y -o Dpkg::Progress-Fancy=0 -o APT::Color=0 -o Dpkg::Use-Pty=0 --no-install-recommends \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN echo "Compiling mongodb c driver version ${MONGODBCDRIVER_VERSION} ..." && \
    mkdir -p /opt/mongo-c-driver/current && \
    git clone -b ${MONGODBCDRIVER_VERSION} --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}

RUN echo "Patching to disable OCSP..." && \
    sed -i "/#if (OPENSSL_VERSION_NUMBER >= 0x10001000L)/i #define OPENSSL_NO_OCSP 1" /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION}/src/libmongoc/src/mongoc/mongoc-openssl-private.h

RUN echo "Compiling mongocxx-driver version ${MONGODBCDRIVER_VERSION} ..." && \
    cd /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION} && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current . && \
    cd /opt/mongo-c-driver/${MONGODBCDRIVER_VERSION} && make all install && \
    echo "/opt/mongo-c-driver/current/lib" > /etc/ld.so.conf.d/mongoc-driver.conf && \
    ldconfig && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2.pc /usr/lib/pkgconfig/mongoc.pc && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2.pc /usr/lib/pkgconfig/mongoc2.pc && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/mongoc2-static.pc /usr/lib/pkgconfig/mongoc2-static.pc && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson.pc && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson2.pc && \
    ln -sv /opt/mongo-c-driver/current/lib/pkgconfig/bson2-static.pc /usr/lib/pkgconfig/bson2-static.pc

ENV LD_LIBRARY_PATH=/usr/local/lib:/opt/mongo-c-driver/current/lib

RUN echo "Patching mongocxx-driver build..." && \
    mkdir -p /app/subprojects/mongo-cxx-driver/build && \
    echo "${MONGODBCXXDRIVER_VERSION}" > /app/subprojects/mongo-cxx-driver/build/VERSION_CURRENT && \
    echo "${MONGODBCXXDRIVER_VERSION}" > /app/subprojects/mongo-cxx-driver/VERSION_CURRENT

RUN echo "Configuring build..." && \
    meson setup build --prefer-static --default-library=static -Dbuild_environment=container

RUN echo "Compiling application..." && \
    meson compile -C build

RUN echo "Cleaning up..." && \
    rm -rf /opt/mongo-c-driver && \
    rm -rf /opt/boost*

# Runtime stage: distroless debian
FROM gcr.io/distroless/cc-debian13:nonroot

# Copy compiled C++ service and healthcheck script from builder
WORKDIR /app
COPY --from=builder --chown=nonroot:nonroot /app/build/cms /app/cms

# Switch to nonroot user
USER nonroot

EXPOSE 10000

CMD ["/app/cms"]