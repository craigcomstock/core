#!/usr/bin/env bash
set -ex
name=cfengine-agent
if docker ps | grep cfengine-agent; then
  docker stop $name
  docker rm $name
fi
if docker images | grep cfengine-agent; then
  docker rmi $name
fi
docker build -t $name -f Dockerfile-$name .
docker run -d --privileged --name $name $name
docker exec -i $name bash -c "wget https://s3.amazonaws.com/cfengine.packages/quick-install-cfengine-enterprise.sh  && sudo bash ./quick-install-cfengine-enterprise.sh agent"
docker image tag cfengine-agent cfengine-agent:latest
docker image tag cfengine-agent localhost:5001/cfengine/cfengine-agent:latest
docker image push localhost:5001/cfengine/cfengine-agent:latest
