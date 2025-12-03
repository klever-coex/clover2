variable "REGISTRY" { }
variable "VERSION" { }
variable "BUILD_MODE" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.source"   = "https://gitlab.com/coex2/clover2"
    "org.opencontainers.image.licenses" = "MIT"
    "org.opencontainers.image.authors"  = "Lapin Matvey"
    "org.opencontainers.image.version"  = "${VERSION}"
  }
}

# Platforms for deploy
variable "PLATFORMS" {
  default = [
    "linux/amd64",
    "linux/arm64",
  ]
}

# Image tags generator
function "tagged" {
  params = [name]
  result = [
    "${REGISTRY}${name}:${VERSION}",

    # For master build have dirty version and latest tag
    equal("master", BUILD_MODE) ? "${REGISTRY}${name}:latest" : "",

    # For develop build have dirty version tag
    # Only version tag

    # Releases have version and stable tags
    equal("release", BUILD_MODE) ? "${REGISTRY}${name}:stable" : "",
  ]
}

#      ____           ___           __                         __
#     / __/__  ____  / _ \___ ___  / /__  __ ____ _  ___ ___  / /_
#    / _// _ \/ __/ / // / -_) _ \/ / _ \/ // /  ' \/ -_) _ \/ __/
#   /_/  \___/_/   /____/\__/ .__/_/\___/\_, /_/_/_/\__/_//_/\__/
#                          /_/          /___/

target "clover2-web" {
  dockerfile = item.dockerfile
  name = "clover2-${item.tgt}"
  target = item.tgt
  tags = item.tags

  context = "."
  labels = LABELS
  output = ["type=registry"]
  platforms = "${PLATFORMS}"

  matrix = {
    item = [
      {
        dockerfile = "docker/frontend/Dockerfile"
        tgt = "gui"
        tags = tagged("clover2-gui")
      },
      {
        dockerfile = "docker/docs/Dockerfile"
        tgt = "docs"
        tags = tagged("clover2-docs")
      }
    ]
  }
}

#    ______          ___
#   /_  __/__  ___  / (_)__  ___ _
#    / / / _ \/ _ \/ / / _ \/ _ `/
#   /_/  \___/\___/_/_/_//_/\_, /
#                          /___/

target "clover2-builder" {
  context = "."
  dockerfile = "docker/builder/Dockerfile"
  labels = LABELS
  output = ["type=registry"]
  tags = tagged("clover2-builder")
}
