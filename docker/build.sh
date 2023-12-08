#!/usr/bin/env bash -ex
name=cfengine-enterprise-hub
version=3.21.2
docker stop $name || true
docker rm $name || true
docker rmi $name || true
docker build -t $name -f Dockerfile-$name .
docker run -d --privileged --name $name $name
docker exec -i $name bash -c "sha256sum --check /quick-install-cfengine-enterprise.sh.sha256"
docker exec -i $name bash -c "CFEngine_Enterprise_Package_Version=$version bash /quick-install-cfengine-enterprise.sh hub"
docker exec -i $name bash -c "rm -rf /var/cfengine/ppkeys/localhost*" # so that new images don't all have the same CFEngine IDs
docker commit $name cfengine/$name:$version
