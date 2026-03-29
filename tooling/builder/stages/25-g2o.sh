G2O_VERSION="502c40774af0bdd404678e8eb12ff08ee5d246ec"

g2o_install() {
    log_info "Install g2o deps"
    sudo apt-get install -y \
        cmake \
        libeigen3-dev \
        libspdlog-dev \
        libsuitesparse-dev \
        ninja-build

    log_info "Clone g2o project (ver: $G2O_VERSION)"
    G2O_SOURCE_DIR=$(mktemp -d --suffix="-g2o")
    git clone https://github.com/RainerKuemmerle/g2o.git $G2O_SOURCE_DIR
    cd $G2O_SOURCE_DIR
    git checkout --detach $G2O_VERSION

    log_info "Build g2o"
    cmake -DG2O_USE_OPENGL=OFF -DG2O_BUILD_SLAM3D_TYPES=ON -DG2O_BUILD_BENCHMARKS=OFF -DG2O_BUILD_EXAMPLES=OFF -DG2O_BUILD_APPS=OFF -DG2O_INSTALL_CMAKE_CONFIG=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .
    make -j$(( $(nproc) / 2 ))

    log_info "Create .deb package"
    DESTDIR=$(pwd)/g2o make install
    mkdir -p g2o/DEBIAN
    cat <<EOF > g2o/DEBIAN/control
Package: g2o
Version: 0.1
Section: libs
Priority: optional
Architecture: arm64
Maintainer: RainerKuemmerle
Description: g2o library build
EOF
    dpkg-deb --build g2o

    log_info "Install .deb package"
    sudo apt install ./g2o.deb -y
    cp ./g2o.deb /home/$USER/.clover2_backup/deb
}

g2o_install
