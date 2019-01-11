#!/usr/bin/env python3
import sys
from collections import OrderedDict

import fabric
from paramiko.ssh_exception import AuthenticationException
from invoke.exceptions import UnexpectedExit

from cf_remote.utils import os_release, column_print, pretty
from cf_remote import log


def ssh_cmd(connection, cmd):
    try:
        log.debug("Running over SSH: '{}'".format(cmd))
        result = connection.run(cmd, hide=True)
        output = result.stdout.strip()
        log.debug("'{}' -> '{}'".format(cmd, output))
        return output
    except UnexpectedExit:
        return None


def ssh_sudo(connection, cmd):
    try:
        log.debug("Running(sudo) over SSH: '{}'".format(cmd))
        result = connection.sudo(cmd, hide=True)
        output = result.stdout.strip()
        log.debug("'{}' -> '{}'".format(cmd, output))
        return output
    except UnexpectedExit:
        return None


def print_info(data):
    log.debug("JSON data from host info: \n" + pretty(data))
    output = OrderedDict()
    print()
    print(data["ssh"])
    os_release = data["os_release"]
    os = like = None
    if os_release:
        if "ID" in os_release:
            os = os_release["ID"]
        if "ID_LIKE" in os_release:
            like = os_release["ID_LIKE"]
    if not os:
        os = data["uname"]
    if os and like:
        output["OS"] = "{} ({})".format(os, like)
    elif os:
        output["OS"] = "{}".format(os)
    else:
        output["OS"] = "Unknown"

    if "arch" in data:
        output["Architecture"] = data["arch"]

    agent_version = data["agent_version"]
    if agent_version:
        output["CFEngine"] = agent_version
    else:
        output["CFEngine"] = "Not installed"

    binaries = []
    if "bin" in data:
        for key in data["bin"]:
            binaries.append(key)
    if binaries:
        output["Binaries"] = ", ".join(binaries)

    column_print(output)
    print()


def connect(host, users=None):
    if "@" in host:
        parts = host.split("@")
        assert len(parts) == 2
        host = parts[1]
        if not users:
            users = [parts[0]]
    if not users:
        users = ["ubuntu", "centos", "vagrant", "root"]
    for user in users:
        try:
            c = fabric.Connection(host=host, user=user)
            c.ssh_user = user
            c.ssh_host = host
            c.run("whoami", hide=True)
            return c
        except AuthenticationException:
            continue
    sys.exit("Could not ssh into {}".format(host))


def get_info(host, users=None, connection=None):
    if not connection:
        with connect(host, users) as c:
            return get_info(host, users, c)

    user, host = connection.ssh_user, connection.ssh_host
    data = OrderedDict()
    data["ssh_user"] = user
    data["ssh_host"] = host
    data["ssh"] = "{}@{}".format(user, host)
    data["whoami"] = ssh_cmd(connection, "whoami")
    data["uname"] = ssh_cmd(connection, "uname")
    data["arch"] = ssh_cmd(connection, "uname -m")
    data["os_release"] = os_release(ssh_cmd(connection, "cat /etc/os-release"))
    data["agent_location"] = ssh_cmd(connection, "which cf-agent")
    agent_version = ssh_cmd(connection, "cf-agent --version")
    if agent_version:
        # 'CFEngine Core 3.12.1 \n CFEngine Enterprise 3.12.1'
        #                ^ split and use this part for version number
        agent_version = agent_version.split()[2]
    data["agent_version"] = agent_version
    data["bin"] = {}
    for bin in ["dpkg", "rpm", "yum", "apt", "pkg"]:
        path = ssh_cmd(connection, "which {}".format(bin))
        if path:
            data["bin"][bin] = path
    return data


def scp(file, remote, connection=None):
    if not connection:
        with connect(remote) as connection:
            scp(file, remote, connection)
    else:
        connection.put(file)
