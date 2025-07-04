# Cross-platform docker image builder

# Build stage: Use pre-built Boost C++ image
FROM rfledesma/boost:latest AS builder

WORKDIR /app
COPY . .

# Install development dependencies
RUN echo "Installing development packages..." && \
    apt update -qqq && apt install -qqq -y --no-install-recommends \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN echo "Compiling mongodb c driver.." && \
    mkdir -p /opt/mongo-c-driver/current && \
    git clone -b 2.0.2 --depth 1 https://github.com/mongodb/mongo-c-driver.git /opt/mongo-c-driver/2.0.2

RUN echo "Patching to disable OCSP..." && \
    sed -i "/#if (OPENSSL_VERSION_NUMBER >= 0x10001000L)/i #define OPENSSL_NO_OCSP 1" /opt/mongo-c-driver/2.0.2/src/libmongoc/src/mongoc/mongoc-openssl-private.h

RUN echo "Compiling mongocxx-driver..." && \
    cd /opt/mongo-c-driver/2.0.2 && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/mongo-c-driver/current . && \
    cd /opt/mongo-c-driver/2.0.2 && make all install && \
    echo "/opt/mongo-c-driver/current" > /etc/ld.so.conf.d/boost.conf && \
    ldconfig

ENV LD_LIBRARY_PATH=/usr/local/lib:/opt/mongo-c-driver/current/lib

RUN echo "Patching mongocxx-driver build..." && \
    mkdir -p /app/subprojects/mongo-cxx-driver/build && \
    echo "4.1.0" > /app/subprojects/mongo-cxx-driver/build/VERSION_CURRENT && \
    echo "4.1.0" > /app/subprojects/mongo-cxx-driver/VERSION_CURRENT

RUN echo "Configuring build..." && \
    meson setup build --prefer-static --default-library=static

RUN echo "Compiling application..." && \
    meson compile -C build

RUN echo "Cleaning up..." && \
    rm -rf /opt/mongo-c-driver/2.0.2

# Runtime stage: Use Debian Bookworm Slim for runtime
FROM debian:bookworm-slim

# Set runtime library path
ENV LD_LIBRARY_PATH=/usr/local/lib

# Install runtime dependencies
RUN echo "Installing runtime packages..." && \
    apt update -qqq && apt install -qqq -y --no-install-recommends \
    openssl \
    ca-certificates \
    libmongoc-1.0-0 \
    libbson-1.0-0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /opt/mongo-c-driver/current/lib /usr/local/lib/
RUN echo "Linking MongoDB C Driver..." && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/mongo.conf \
    && ldconfig

# Create nonroot user and group
RUN echo "Configuring nonroot account..." && \
    groupadd -r nonroot && useradd -r -g nonroot -d /home/nonroot -s /sbin/nologin nonroot && \
    mkdir -p /home/nonroot && chown nonroot:nonroot /home/nonroot

# Copy compiled C++ service and healthcheck script from builder
WORKDIR /app
COPY --from=builder --chown=nonroot:nonroot /app/build/cms /app/

# Set ownership for app directory
RUN echo "Setting application directory permissions..." && \
    chown -R nonroot:nonroot /app

# Switch to nonroot user
USER nonroot

EXPOSE 10000

CMD ["/app/cms"]