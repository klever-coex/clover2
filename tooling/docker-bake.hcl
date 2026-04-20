variable "BUILD_MODE" { }
variable "DOCKER_OUTPUT_DIR" { }
variable "REGISTRY_POLICY" { }
variable "REGISTRY" { }

variable "CLOVER2_VERSION" { }
variable "CLOVER2_BASE_VERSION" { }
variable "CLOVER2_GIT_HASH" { }

variable "LABELS" {
  default = {
    "org.opencontainers.image.authors"  = "Lapin Matvey"
    "org.opencontainers.image.licenses" = "MIT"
    "org.opencontainers.image.source"   = "https://gitlab.com/coex2/clover2"
    "org.opencontainers.image.version"  = CLOVER2_VERSION
    "org.opencontainers.image.revision" = CLOVER2_GIT_HASH
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
  result = compact([
    "${REGISTRY}${name}:${CLOVER2_GIT_HASH}",

    # For master build have dirty version and latest tag
    equal("master", BUILD_MODE) ? "${REGISTRY}${name}:latest" : null,

    # For develop build have dirty version tag
    # Only version tag

    # Releases have version and stable tags
    equal("release", BUILD_MODE) ? "${REGISTRY}${name}:stable" : null,
    equal("release", BUILD_MODE) ? "${REGISTRY}${name}:${CLOVER2_VERSION}" : null,
  ])
}

function "outputs" {
  params = [name, save-tar]
  result = compact([
    REGISTRY_POLICY == "push" ? "type=registry" : null,
    REGISTRY_POLICY == "load" ? "type=docker" : null,
    save-tar ? "type=oci,dest=${DOCKER_OUTPUT_DIR}/${name}.tar" : null,
  ])
}

target "base" {
  context = "."
  labels = LABELS

  cache-from = ["type=local,src=.cache/docker"]
  cache-to   = ["type=local,dest=.cache/docker,mode=max"]
}

#      ____           ___           __                         __
#     / __/__  ____  / _ \___ ___  / /__  __ ____ _  ___ ___  / /_
#    / _// _ \/ __/ / // / -_) _ \/ / _ \/ // /  ' \/ -_) _ \/ __/
#   /_/  \___/_/   /____/\__/ .__/_/\___/\_, /_/_/_/\__/_//_/\__/
#                          /_/          /___/

target "project-deploy" {
  dockerfile = item.dockerfile
  name = item.tgt
  tags = tagged(item.tgt)
  output = outputs(item.tgt, true)

  inherits = ["base"]
  platforms = PLATFORMS

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
      {
        dockerfile = "docker/ros/Dockerfile"
        tgt = "clover2-ros"
      }
    ]
  }
}

target "mirror" {
  tags = tagged("clover2-wetty")
  output = outputs("clover2-wetty", true)

  inherits = ["base"]
  platforms = PLATFORMS

  dockerfile-inline = <<EOF
  FROM wettyoss/wetty
  EOF
}

#       ____  ____  _____
#      / __ \/ __ \/ ___/
#     / /_/ / / / /\__ \
#    / _, _/ /_/ /___/ /
#   /_/ |_|\____//____/

target "ros" {
  dockerfile = "docker/ros/Dockerfile"
  name = tgt
  tags = tagged(tgt)
  output = outputs(tgt, true)
  target = tgt

  inherits = ["base"]
  platforms = PLATFORMS

  matrix = {
    tgt = [ "clover2-ros-dev", "clover2-ros" ]
  }
}

#    ______          ___
#   /_  __/__  ___  / (_)__  ___ _
#    / / / _ \/ _ \/ / / _ \/ _ `/
#   /_/  \___/\___/_/_/_//_/\_, /
#                          /___/

target "builder" {
  dockerfile = item.dockerfile
  name = item.tgt
  tags = tagged(item.tgt)
  output = outputs(item.tgt, false)

  inherits = ["base"]

  matrix = {
    item = [
      {
        dockerfile = "docker/builder/Dockerfile"
        tgt = "clover2-builder"
      }
    ]
  }
}

#       ____ _  ____ __                                       __
#      / __ \ |/ / // /    _______  ______  ____  ____  _____/ /_
#     / /_/ /   / // /_   / ___/ / / / __ \/ __ \/ __ \/ ___/ __/
#    / ____/   /__  __/  (__  ) /_/ / /_/ / /_/ / /_/ / /  / /_
#   /_/   /_/|_| /_/    /____/\__,_/ .___/ .___/\____/_/   \__/
#                                 /_/   /_/

target "_clover2-px4" {
  dockerfile = "docker/px4/Dockerfile"
  output = compact([
    REGISTRY_POLICY == "push" ? "type=registry" : null,
    REGISTRY_POLICY == "load" ? "type=docker" : null,
  ])

  inherits = ["base"]
}

target "clover2-px4-deps" {
  inherits = ["_clover2-px4"]
  target = "clover2-px4-deps"
  tags = tagged("clover2-px4-deps")
}

target "clover2-px4-dev" {
  inherits = ["_clover2-px4"]
  target = "clover2-px4-dev"
  tags = tagged("clover2-px4-dev")
}

target "clover2-px4-sitl" {
  inherits = ["_clover2-px4"]
  target = "clover2-px4-sitl"
  tags = tagged("clover2-px4-sitl")
}

#      ___       _ __   __  _____
#     / _ )__ __(_) /__/ / / ___/______  __ _____  ___
#    / _  / // / / / _  / / (_ / __/ _ \/ // / _ \(_-<
#   /____/\_,_/_/_/\_,_/  \___/_/  \___/\_,_/ .__/___/
#                                          /_/

group "all" {
  targets = ["web", "tooling", "ros", "px4"]
}

group "tooling" {
  targets = ["builder"]
}

group "web" {
  targets = ["clover2-docs", "clover2-gui", "mirror"]
}

group "ros" {
  targets = ["clover2-ros"]
}

group "px4" {
  targets = ["clover2-px4-deps", "clover2-px4-dev", "clover2-px4-sitl"]
}
