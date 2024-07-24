# note: you MUST start the hub first, for the agent to bootstrap to it
docker run --name cfengine-hub cfengine-hub
#docker run --name cfengine-agent cfengine-agent
docker run -d --privileged -v /Users/craig/cfe:/data --name cfengine-agent cfengine-agent
