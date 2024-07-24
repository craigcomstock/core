#!/usr/bin/env bash
set -ex
name=cfengine-hub
docker stop $name
docker rm $name
docker rmi $name
docker build -t $name -f Dockerfile-$name .
docker run -d --privileged --name $name $name
docker exec -i $name bash -c "wget https://s3.amazonaws.com/cfengine.packages/quick-install-cfengine-enterprise.sh  && sudo bash ./quick-install-cfengine-enterprise.sh hub"
docker ...
