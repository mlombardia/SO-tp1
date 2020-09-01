#!/bin/bash
docker run -v "${PWD}:/root" --cap-add=SYS_PTRACE --security-opt seccomp:unconfined -ti agodio/itba-so:1.0
