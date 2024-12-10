#!/usr/bin/env bash
PWD=$(dirname "$0")
. "$PWD"/detect-environment.sh
names="${DISTRO}-${RELEASE}-${ARCH}.sh ${DISTRO}-${RELEASE}.sh ${DISTRO}.sh"
for name in $names; do
  if [ -f "$name" ]; then
    echo "Running dependency script ${name}..."
    ./"$name"
    exit 0
  else
    echo "No dependency script ${name}"
  fi
done
echo "Couldn't find a dependencies script this platform. Please submit one."
