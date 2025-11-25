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
    ]
}

function "tagged_with_latest" {
    params = [name]
    result = [
      "${REGISTRY}${name}:${VERSION}",
      "${REGISTRY}${name}:latest",
    ]
}

group "web" {
  targets = ["clover2-gui", "clover2-docs"]
}

group "web-dev" {
  targets = ["clover2-gui-dev", "clover2-docs-dev"]
}

target "clover2-gui_base" {
    context = "."
    dockerfile = "docker/frontend/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    output = ["type=registry"]
}

target "clover2-docs_base" {
    context = "."
    dockerfile = "docker/docs/Dockerfile"
    platforms = "${PLATFORMS}"
    labels = LABELS
    output = ["type=registry"]
}

target "clover2-gui" {
    inherits = ["clover2-gui_base"]
    tags = tagged_with_latest("clover2-gui")
}

target "clover2-docs" {
    inherits = ["clover2-docs_base"]
    tags = tagged_with_latest("clover2-docs")
}

target "clover2-gui-dev" {
    inherits = ["clover2-gui_base"]
    tags = tagged("clover2-gui")
}

target "clover2-docs-dev" {
    inherits = ["clover2-docs_base"]
    tags = tagged("clover2-docs")
}
