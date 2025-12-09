#!/usr/bin/env python3

import os

ENVS_FOR_PASS = [
    "BUILD_MODE",
    "REGISTRY_HOST",
    "REGISTRY",
    "REGISTRY_POLICY",
    "DOCKER_REGISTRY_USER",
    "DOCKER_REGISTRY_PASSWORD"
]

def parse_env(names):
    envs_map = {name: os.environ.get(name) for name in names if os.environ.get(name)}
    return envs_map

if __name__ == "__main__":
    a = parse_env(ENVS_FOR_PASS)
    
    for key, val in a.items():
        print(f'{key}: {val}')
