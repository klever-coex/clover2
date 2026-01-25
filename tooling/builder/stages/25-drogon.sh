DROGON_VERSION="a22956b82b6b221ceeff83913c3014ce0d048555"

drogon_install() {
    log_info "Clone Drogon project (ver: $DROGON_VERSION)"
    DROGON_SOURCE_DIR=$(mktemp -d --suffix="-drogon")
    git clone https://github.com/drogonframework/drogon.git "$DROGON_SOURCE_DIR"
    cd "$DROGON_SOURCE_DIR"
    git checkout --detach "$DROGON_VERSION"
    git submodule update --init --recursive

    log_info "Build and install Drogon"
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON  -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
    make -j$(nproc)

    log_info "Installing Drogon"
    sudo make install
}

drogon_install
