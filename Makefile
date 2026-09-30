.PHONY: help clean
.DEFAULT_GOAL := help

-include .env
export

# Configuration
BUILD_MODE ?= develop
REGISTRY ?= $(REGISTRY_HOST)/klever-coex/clover2/
REGISTRY_HOST ?= ghcr.io
REGISTRY_POLICY ?= load
PROJECT_DIR ?= $(shell pwd)
BUILD_EXPTRAS_DIR ?= $(PROJECT_DIR)/build-extras
DOCKER_OUTPUT_DIR ?= $(BUILD_EXPTRAS_DIR)/docker

# Constants
UID ?= $(shell id -u)
GID ?= $(shell id -g)

TOOLING ?= clover2
COMPOSE := $(TOOLING) version compose --mode $(BUILD_MODE)
CLOVER2_GIT_HASH := $(shell $(COMPOSE) --field git_hash)
CLOVER2_VERSION := $(shell $(COMPOSE) --field version)

ifeq ($(strip $(CLOVER2_VERSION)),)
$(error clover2 version compose failed (check BUILD_MODE '$(BUILD_MODE)' and that the clover2 CLI is installed))
endif

export CLOVER2_VERSION
export REGISTRY
export BUILD_MODE
export PROJECT_DIR
export DOCKER_OUTPUT_DIR
export LOCAL_CACHE = 1

## help: Show this help message
help:
	@printf "Available targets:\n\n"
	@awk '/^[a-zA-Z\-_0-9%:\\]+/ { \
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

## clover2-bake-%: Build docker images using buildx bake
clover2-bake-%:
	docker buildx bake \
		$(if $(TARGET_ARCH),--set *.platform=linux/$(TARGET_ARCH)) \
		-f docker/docker-bake.hcl \
		--progress plain \
		$*

## clover2-bake-push-%: Push docker images
clover2-bake-push-%:
	docker buildx bake --set *.output=type=registry -f docker/docker-bake.hcl $*

## clover2-bake-print-%: Print buildx bake configuration
clover2-bake-print-%:
	docker buildx bake -f docker/docker-bake.hcl --print $*

## clover2-bake-save-%: Save docker images to tar files
clover2-bake-save-%:
	@mkdir -p $(DOCKER_OUTPUT_DIR)
	docker buildx bake \
		$(if $(TARGET_ARCH),--set *.platform=linux/$(TARGET_ARCH)) \
		-f docker/docker-bake.hcl \
		--progress plain \
		--set *.output=type=docker,dest=$(DOCKER_OUTPUT_DIR)/$*.tar \
		$*

## clover2-docs-%: Execute commands from docs dir
clover2-docs-%:
	$(MAKE) -C $(PROJECT_DIR)/docs $*

## clover2-docs-doxygen: Generate doxygen output
clover2-docs-doxygen:
	mkdir -p $(PROJECT_DIR)/docs/build/doxygen
	doxygen

## clover2-frontend-%: Execute npm run command in frontend folder
clover2-frontend-%:
	cd $(PROJECT_DIR)/frontend && npm run $*

## clover2-devtool-install-repos: install mainline ros2 repos to third party folder
clover2-devtool-install-repos:
	vcs import third_party < third_party/clover2.repos

## clean: Cleanup build artifacts
clean:
	rm -rf $(PROJECT_DIR)/build-* $(PROJECT_DIR)/docs/build

## version: Show current version information
version:
	@echo "BUILD_MODE: $(BUILD_MODE)"
	@echo "CLOVER2_GIT_HASH: $(CLOVER2_GIT_HASH)"
	@echo "CLOVER2_VERSION: $(CLOVER2_VERSION)"
	@echo "REGISTRY: $(REGISTRY)"
