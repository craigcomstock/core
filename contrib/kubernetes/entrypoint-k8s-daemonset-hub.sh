#!/bin/bash

set -x

export CALLED_FROM_INITSCRIPT=1
source /var/cfengine/bin/cfengine3-nova-hub-init-d.sh
start_postgres
start_httpd

CFENGINE_HUB_IP=$(kubectl describe pod -l app=cfengine-hub --namespace=cfengine | grep ^IP: | awk {'print $2'})
/var/cfengine/bin/cf-agent --bootstrap ${CFENGINE_HUB_IP} --log-level info

# TODO: to manage this properly we probably need to run the daemons in separate containers within a pod for the livecycle management by Kubernetes
for process in cf-execd cf-monitord cf-serverd cf-hub cf-reactor; do
    echo "${process} is not running. Starting..."
    /var/cfengine/bin/${process}
done

while true; do sleep 3600; done
