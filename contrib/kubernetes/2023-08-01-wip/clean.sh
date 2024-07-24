# clean up docker stuff
for name in cfengine-hub cfengine-agent; do
  docker stop $name
  docker rm $name
  docker rmi $name
done
