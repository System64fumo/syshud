FROM debian:13-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    libgtk4-layer-shell-dev \
    libgtkmm-4.0-dev \
    libpulse-dev \
    libevdev-dev \
    libwireplumber-0.5-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
