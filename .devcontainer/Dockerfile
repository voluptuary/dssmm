# devkitPro publishes this image with devkitARM and the GBA rules preinstalled,
# avoiding the blocked package bootstrap/download endpoints in this Codespace.
FROM devkitpro/devkitarm:20260610

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM

# Dependencies for the target host verifiers; no cartridge protocols are shared.
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc git make python3 python3-unicorn python3-setuptools nodejs \
    && rm -rf /var/lib/apt/lists/*
