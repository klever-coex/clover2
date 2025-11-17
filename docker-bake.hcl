variable "REGISTRY" { }
variable "GIT_COMMIT" { }
variable "VERSION" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.source"   = "https://gitlab.com/l_motya/clover2"
    "org.opencontainers.image.licenses" = "MIT"
  }
}

variable "PLATFORMS" {
  default = [
    "linux/amd64",
    "linux/arm64",
  ]
}

function "tagged" {
    params = [name]
    result = [
      "${REGISTRY}${name}:${VERSION}",
      "${REGISTRY}${name}:${GIT_COMMIT}"
      ]
}

group "build" {
  targets = ["clover2-gui", "clover2-docs"]
}

target "clover2-gui" {
    context = "."
    dockerfile = "docker/frontend/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    tags = tagged("clover2-gui")
}

target "clover2-docs" {
    context = "."
    dockerfile = "docker/docs/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    tags = tagged("clover2-docs")
}
