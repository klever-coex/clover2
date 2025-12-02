variable "REGISTRY" { }
variable "GIT_COMMIT" { }
variable "VERSION" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.source"   = "https://gitlab.com/coex2/clover2"
    "org.opencontainers.image.licenses" = "MIT"
    "org.opencontainers.image.authors"  = "Lapin Matvey"
    "org.opencontainers.image.version"  = "${VERSION}"
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

#      ___       _ __   __  _____
#     / _ )__ __(_) /__/ / / ___/______  __ _____  ___
#    / _  / // / / / _  / / (_ / __/ _ \/ // / _ \(_-<
#   /____/\_,_/_/_/\_,_/  \___/_/  \___/\_,_/ .__/___/
#                                          /_/

group "web" {
  targets = ["clover2-gui", "clover2-docs"]
}

group "web-dev" {
  targets = ["clover2-gui-dev", "clover2-docs-dev"]
}

group "tooling" {
  targets = ["clover2-builder"]
}

group "tooling-dev" {
  targets = ["clover2-builder-dev"]
}

#      ____           ___           __                         __
#     / __/__  ____  / _ \___ ___  / /__  __ ____ _  ___ ___  / /_
#    / _// _ \/ __/ / // / -_) _ \/ / _ \/ // /  ' \/ -_) _ \/ __/
#   /_/  \___/_/   /____/\__/ .__/_/\___/\_, /_/_/_/\__/_//_/\__/
#                          /_/          /___/

target "_common" {
  context = "."
  labels = LABELS
  output = ["type=registry"]
  platforms = "${PLATFORMS}"
}

target "clover2-gui" {
  dockerfile = "docker/frontend/Dockerfile"
  inherits = ["_common"]
  tags = tagged_with_latest("clover2-gui")
}

target "clover2-docs" {
  dockerfile = "docker/docs/Dockerfile"
  inherits = ["_common"]
  tags = tagged_with_latest("clover2-docs")
}

target "clover2-gui-dev" {
  dockerfile = "docker/frontend/Dockerfile"
  inherits = ["_common"]
  tags = tagged("clover2-gui")
}

target "clover2-docs-dev" {
  dockerfile = "docker/docs/Dockerfile"
  inherits = ["_common"]
  tags = tagged("clover2-docs")
}

#    ______          ___
#   /_  __/__  ___  / (_)__  ___ _
#    / / / _ \/ _ \/ / / _ \/ _ `/
#   /_/  \___/\___/_/_/_//_/\_, /
#                          /___/

target "clover2-builder_base" {
  context = "."
  dockerfile = "docker/builder/Dockerfile"
  labels = LABELS
  output = ["type=registry"]
}

target "clover2-builder" {
  inherits = ["clover2-builder_base"]
  tags = tagged_with_latest("clover2-builder")
}

target "clover2-builder-dev" {
  inherits = ["clover2-builder_base"]
  tags = tagged("clover2-builder")
}
