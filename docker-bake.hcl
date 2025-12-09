variable "BUILD_MODE" { }
variable "REGISTRY_POLICY" { }
variable "REGISTRY" { }
variable "CLOVER2_VERSION" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.source"   = "https://gitlab.com/coex2/clover2"
    "org.opencontainers.image.licenses" = "MIT"
    "org.opencontainers.image.authors"  = "Lapin Matvey"
    "org.opencontainers.image.version"  = "${CLOVER2_VERSION}"
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
    "${REGISTRY}${name}:${CLOVER2_VERSION}",

    # For master build have dirty version and latest tag
    equal("master", BUILD_MODE) ? "${REGISTRY}${name}:latest" : "",

    # For develop build have dirty version tag
    # Only version tag

    # Releases have version and stable tags
    equal("release", BUILD_MODE) ? "${REGISTRY}${name}:stable" : "",
  ]
}

target "_output" {
  output = ["type=registry"]
}

#      ___       _ __   __  _____
#     / _ )__ __(_) /__/ / / ___/______  __ _____  ___
#    / _  / // / / / _  / / (_ / __/ _ \/ // / _ \(_-<
#   /____/\_,_/_/_/\_,_/  \___/_/  \___/\_,_/ .__/___/
#                                          /_/

group "all" {
  targets = ["clover2-deploy", "clover2-builder"]
}

group "clover2-tooling" {
  targets = ["clover2-builder"]
}

#      ____           ___           __                         __
#     / __/__  ____  / _ \___ ___  / /__  __ ____ _  ___ ___  / /_
#    / _// _ \/ __/ / // / -_) _ \/ / _ \/ // /  ' \/ -_) _ \/ __/
#   /_/  \___/_/   /____/\__/ .__/_/\___/\_, /_/_/_/\__/_//_/\__/
#                          /_/          /___/

target "clover2-deploy" {
  dockerfile = item.dockerfile
  name = "${item.tgt}"
  tags = item.tags

  inherits = ["_output"]

  context = "."
  labels = LABELS
  platforms = "${PLATFORMS}"

  matrix = {
    item = [
      {
        dockerfile = "docker/docs/Dockerfile"
        tgt = "clover2-docs"
        tags = tagged("clover2-docs")
      },
      {
        dockerfile = "docker/frontend/Dockerfile"
        tgt = "clover2-gui"
        tags = tagged("clover2-gui")
      },
      {
        dockerfile = "docker/ros/Dockerfile"
        tgt = "clover2-ros"
        tags = tagged("clover2-ros")
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
  inherits = ["_output"]
  tags = tagged("clover2-builder")
}
