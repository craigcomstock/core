#!/bin/bash

set -x

# mount FSs from the host VM to /chroot folder
for i in proc sys run dev; do mkdir -p /chroot/$i; done
mount --types proc /rootproc /chroot/proc
mount --rbind /rootsys /chroot/sys
mount --make-rslave /chroot/sys
mount --rbind /rootdev /chroot/dev
mount --make-rslave /chroot/dev
mount --bind /rootrun /chroot/run
mount --make-slave /chroot/run
for i in bin lib lib64 sbin; do cp -a /rootfs/$i /chroot/; done # for the POC it's hardcoded for Amazon Linux for now
for i in $(ls -1 /rootfs/ | grep -v -E "bin|dev|lib|lib64|proc|run|sbin|sys"); do mkdir /chroot/$i && mount -o bind /rootfs/$i /chroot/$i; done

CFENGINE_HUB_IP=$(kubectl describe pod -l app=cfengine-hub | grep ^IP: | awk {'print $2'})

# entrypoint script for installing and running the agent to be executed from chroot jail
cat > /chroot/entry.sh << EOF
#!/bin/bash

set -x

cd /tmp
# this is a little bit dirty because it's executed in chroot and in fact installed on the host OS instead of a container
# in a good way would be to mount it from a container to the chroot, but managing different OSs is significant amount of work
# which is out of scope for the POC
if [ ! -f /var/cfengine/bin/cf-agent ]; then
    wget https://s3.amazonaws.com/cfengine.packages/quick-install-cfengine-enterprise.sh && bash ./quick-install-cfengine-enterprise.sh agent
fi

/var/cfengine/bin/cf-agent --bootstrap ${CFENGINE_HUB_IP} --log-level info

# TODO: to manage this properly we probably need to use 'init' container to install the agent and then run
#       the daemons in separate containers within a pod for the livecycle management by Kubernetes
for process in cf-execd cf-monitord cf-serverd; do
    echo "\${process} is not running. Starting..."
    /var/cfengine/bin/\${process}
done

# TODO: hub pod can change IP, so we heed to implement a logic for checking and refreshing it

while true; do sleep 3600; done
EOF
chmod +x /chroot/entry.sh

chroot /chroot/ /entry.sh
