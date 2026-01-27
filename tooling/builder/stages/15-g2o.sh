G2O_VERSION="e3b127f8d49ef5b597ad5107d6d6d958e2393490"

g2o_install() {
    log_info "Clone g2o project (ver: $G2O_VERSION)"

    G2O_SOURCE_DIR=$(mktemp -d --suffix="-g2o")
    git clone https://github.com/RainerKuemmerle/g2o.git $G2O_SOURCE_DIR
    cd $G2O_SOURCE_DIR
    git checkout --detach $G2O_VERSION

    log_info "Build g2o"
    mkdir build
    cd $G2O_SOURCE_DIR/build
    cmake -DG2O_BUILD_APPS=OFF -DG2O_BUILD_EXAMPLES=OFF -DG2O_USE_OPENGL=OFF -DG2O_BUILD_SLAM2D_TYPES=OFF -DG2O_BUILD_SLAM3D_TYPES=OFF -DCMAKE_BUILD_TYPE=Release ..
    make -j

    log_info "Install g2o"
    sudo make install
}

g2o_install
