.PHONY: help clean
.DEFAULT_GOAL := help

-include .env
export

# Configuration
BUILD_MODE ?= develop
REGISTRY_HOST ?= registry.gitlab.com
REGISTRY ?= $(REGISTRY_HOST)/coex2/clover2/
PROJECT_DIR ?= $(shell pwd)
DOCKER_OUTPUT_DIR ?= $(PROJECT_DIR)/build-docker

# Constants
UID ?= $(shell id -u)
GID ?= $(shell id -g)

# Calculate CLOVER2_VERSION based on BUILD_MODE
CLOVER2_BASE_VERSION := $(shell cat $(PROJECT_DIR)/tooling/VERSION 2>/dev/null)
CLOVER2_GIT_HASH := $(shell git -C $(PROJECT_DIR) rev-parse --short HEAD 2>/dev/null || echo "unknown")

ifeq ($(BUILD_MODE),release)
	CLOVER2_VERSION := $(CLOVER2_BASE_VERSION)
else ifeq ($(BUILD_MODE),master)
	CLOVER2_VERSION := $(CLOVER2_BASE_VERSION)+$(CLOVER2_GIT_HASH)
else ifeq ($(BUILD_MODE),develop)
	CLOVER2_VERSION := $(CLOVER2_BASE_VERSION)+$(CLOVER2_GIT_HASH)
else
	$(error Unknown BUILD_MODE '$(BUILD_MODE)'. Expected one of: release, master, develop)
endif

export CLOVER2_VERSION
export REGISTRY
export BUILD_MODE
export PROJECT_DIR
export DOCKER_OUTPUT_DIR

## help: Show this help message
help:
	@printf "Available targets:\n\n"
	@awk '/^[a-zA-Z\-\_0-9%:\\]+/ { \
		helpMessage = match(lastLine, /^## (.*)/); \
		if (helpMessage) { \
		helpCommand = $$1; \
		helpMessage = substr(lastLine, RSTART + 3, RLENGTH); \
		gsub("\\\\", "", helpCommand); \
		gsub(":+$$", "", helpCommand); \
		printf "  \x1b[32;01m%-35s\x1b[0m %s\n", helpCommand, helpMessage; \
		} \
	} \
	{ lastLine = $$0 }' $(MAKEFILE_LIST) | sort -u
	@printf "\n"

## docker-bake-%: Build docker images using docker buildx bake
docker-bake-%:
	@mkdir -p $(DOCKER_OUTPUT_DIR)
	docker buildx bake -f tooling/docker-bake.hcl --progress=plain $*

## docker-print: Print docker buildx bake configuration
docker-print-%:
	docker buildx bake --print $*

## builder-download: Download base disk image for builder
builder-download:
	$(PROJECT_DIR)/tooling/builder/download.py \
		--configuration clover2-ubuntu24 \
		--output $(PROJECT_DIR)/build-clover2-image/clover2-$(CLOVER2_VERSION).img

## builder-build: Build the clover2 image locally
builder-build:
	$(PROJECT_DIR)/tooling/builder/builder.py \
		--sudo \
		--output $(PROJECT_DIR)/build-clover2-image/clover2-$(CLOVER2_VERSION).img

## builder-image-setup: Setup image with secret server
builder-image-setup: version
	/bin/bash $(PROJECT_DIR)/tooling/builder/image-setup.sh

## builder-in-docker: Run any builder task inside the clover2-builder docker image
builder-%-in-docker:
	docker run \
		--rm \
		-i \
		--net=host \
		--privileged \
		--env REGISTRY=$(REGISTRY) \
		--env BUILD_MODE=$(BUILD_MODE) \
		-v /dev:/dev \
		-v $(PROJECT_DIR):/builder \
		-w /builder \
		$(REGISTRY)clover2-builder:$(CLOVER2_GIT_HASH) \
		sh -c "make builder-$*"

clover2-devtool-install-repos:
	vcs import third_party < third_party/clover2.repos

## clean: Cleanup build artifacts
clean:
	rm -rf $(PROJECT_DIR)/build-* $(PROJECT_DIR)/docs/build

## version: Show current version information
version:
	@echo "BUILD_MODE: $(BUILD_MODE)"
	@echo "CLOVER2_BASE_VERSION: $(CLOVER2_BASE_VERSION)"
	@echo "CLOVER2_GIT_HASH: $(CLOVER2_GIT_HASH)"
	@echo "CLOVER2_VERSION: $(CLOVER2_VERSION)"
	@echo "REGISTRY: $(REGISTRY)"
