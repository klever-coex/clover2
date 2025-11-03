variable "REGISTRY" { default = "localhost:5000/" }

variable "TAG" { default = "latest" }

variable "LABELS" {
  default = {
    "org.opencontainers.image.source"   = "https://github.com/l_motya/clover2"
    "org.opencontainers.image.licenses" = "MIT"
  }
}

variable "PLATFORMS" {
  default = [
    "linux/amd64",
    "linux/arm64",
  ]
}

function "tag" {
    params = [name]
    result = ["${REGISTRY}${name}:${TAG}"]
}

target "clover2-gui" {
    context = "."
    dockerfile = "docker/frontend/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    tags = tag("clover2-gui")
}

target "clover2-docs" {
    context = "."
    dockerfile = "docker/docs/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    tags = tag("clover2-docs")
}
