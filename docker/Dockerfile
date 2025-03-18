FROM ubuntu:22.04 AS build

# Install dependencies
RUN apt-get update && apt-get install -y \
    git \
    gcc \
    g++ \
    cmake \
    make \
    libnuma-dev \
    libssl-dev \
    libcrypto++-dev \
    libcap-dev \
    python3 \
    python3-pip \
    pkg-config \
    libaio-dev \
    ninja-build \
    libcunit1-dev \
    && rm -rf /var/lib/apt/lists/*

# Install SPDK dependencies
RUN pip3 install meson pyelftools

# Set up working directory
WORKDIR /app

# Clone SPDK (specific version for stability)
RUN git clone --recurse-submodules https://github.com/spdk/spdk.git
WORKDIR /app/spdk
RUN git checkout v23.05

# Configure and build SPDK
RUN ./configure --disable-tests --disable-unit-tests --disable-examples
RUN make -j$(nproc)

# Now build our NVMe-oF benchmarking suite
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libnuma1 \
    libssl3 \
    libcrypto++8 \
    libcap2 \
    libaio1 \
    libcunit1 \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Copy SPDK from the build stage
COPY --from=build /app/spdk /opt/spdk

# Set environment variables
ENV SPDK_DIR=/opt/spdk
ENV LD_LIBRARY_PATH=/opt/spdk/build/lib:$LD_LIBRARY_PATH

# Set up working directory
WORKDIR /app

# Copy our application
COPY . /app

# Build our application
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Set the entrypoint
ENTRYPOINT ["/app/build/bin/nvmeof_benchmarking"]
CMD ["--help"]