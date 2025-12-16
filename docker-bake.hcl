variable "BUILD_MODE" { }
variable "DOCKER_OUTPUT_DIR" { }
variable "REGISTRY_POLICY" { }
variable "REGISTRY" { }
variable "CLOVER2_VERSION" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.authors"  = "Lapin Matvey"
    "org.opencontainers.image.licenses" = "MIT"
    "org.opencontainers.image.source"   = "https://gitlab.com/coex2/clover2"
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

function "outputs" {
  params = [name]
  result = [
    "type=registry",
    "type=oci,dest=${DOCKER_OUTPUT_DIR}/${name}.tar"
  ]
}

#      ___       _ __   __  _____
#     / _ )__ __(_) /__/ / / ___/______  __ _____  ___
#    / _  / // / / / _  / / (_ / __/ _ \/ // / _ \(_-<
#   /____/\_,_/_/_/\_,_/  \___/_/  \___/\_,_/ .__/___/
#                                          /_/

group "all" {
  targets = ["deploy", "tooling"]
}

group "tooling" {
  targets = ["builder"]
}

group "deploy" {
  targets = ["project-deploy", "mirror-wetty"]
}

#      ____           ___           __                         __
#     / __/__  ____  / _ \___ ___  / /__  __ ____ _  ___ ___  / /_
#    / _// _ \/ __/ / // / -_) _ \/ / _ \/ // /  ' \/ -_) _ \/ __/
#   /_/  \___/_/   /____/\__/ .__/_/\___/\_, /_/_/_/\__/_//_/\__/
#                          /_/          /___/

target "project-deploy" {
  dockerfile = item.dockerfile
  name = "${item.tgt}"
  tags = tagged(item.tgt)
  output = outputs(item.tgt)

  context = "."
  labels = LABELS
  platforms = "${PLATFORMS}"

  matrix = {
    item = [
      {
        dockerfile = "docker/docs/Dockerfile"
        tgt = "clover2-docs"
      },
      {
        dockerfile = "docker/frontend/Dockerfile"
        tgt = "clover2-gui"
      },
      # {
      #   dockerfile = "docker/ros/Dockerfile"
      #   tgt = "clover2-ros"
      # }
    ]
  }
}

target "mirror-wetty" {
  tags = tagged("clover2-wetty")
  output = outputs("clover2-wetty")

  context = "."
  platforms = "${PLATFORMS}"

  dockerfile-inline = <<EOF
  FROM wettyoss/wetty
  EOF
}

#    ______          ___
#   /_  __/__  ___  / (_)__  ___ _
#    / / / _ \/ _ \/ / / _ \/ _ `/
#   /_/  \___/\___/_/_/_//_/\_, /
#                          /___/

target "builder" {
  dockerfile = item.dockerfile
  name = "${item.tgt}"
  tags = tagged(item.tgt)
  output = ["type=registry"]

  context = "."
  labels = LABELS

  matrix = {
    item = [
      {
        dockerfile = "docker/builder/Dockerfile"
        tgt = "clover2-builder"
      }
    ]
  }
}
