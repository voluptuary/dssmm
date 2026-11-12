# devkitPro publishes this image with devkitARM and the GBA rules preinstalled,
# avoiding the blocked package bootstrap/download endpoints in this Codespace.
FROM devkitpro/devkitarm:20260610

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM

# Dependencies for the target host verifiers; no cartridge protocols are shared.
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc g++ git gh make cmake cppcheck curl unzip zlib1g-dev \
    python3 python3-unicorn python3-setuptools python3-yaml nodejs \
    && rm -rf /var/lib/apt/lists/*
