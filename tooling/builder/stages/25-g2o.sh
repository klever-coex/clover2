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

    G2O_DEB="g2o-git+$(git -C $G2O_SOURCE_DIR rev-parse --short HEAD)"

    log_info "Build g2o"
    cmake -DG2O_USE_OPENGL=OFF -DG2O_BUILD_SLAM3D_TYPES=ON -DG2O_BUILD_BENCHMARKS=OFF -DG2O_BUILD_EXAMPLES=OFF -DG2O_BUILD_APPS=OFF -DG2O_INSTALL_CMAKE_CONFIG=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .
    make -j$(( $(nproc) / 2 ))

    log_info "Create .deb package"
    DESTDIR=$G2O_SOURCE_DIR/$G2O_DEB make install
    mkdir -p $G2O_DEB/DEBIAN
    cat <<EOF > $G2O_DEB/DEBIAN/control
Package: $G2O_DEB
Version: 0.1
Section: libs
Priority: optional
Architecture: arm64
Maintainer: RainerKuemmerle
Description: g2o library build (https://github.com/RainerKuemmerle)
EOF
    dpkg-deb --build $G2O_DEB

    log_info "Install .deb package"
    sudo apt install ./$G2O_DEB.deb -y
    cp ./$G2O_DEB.deb /home/$USER/.clover2_backup/deb
}

g2o_install
