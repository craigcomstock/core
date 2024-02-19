set -ex
test ! -d /var/cfengine
docker stop cfengine-chroot-agent
docker rm cfengine-chroot-agent
docker rmi cfengine-chroot-agent
